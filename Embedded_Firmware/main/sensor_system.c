#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "lcd.h"
#include "servo.h"
#include "mfrc522.h"
#include "mqtt.h"
#include "spi.h"
#include "wifi.h"
#include "dht.h"

#define MAX_SLOTS 3
#define GATE_OPEN_TIME_MS 4000 
#define DHT_TEMP_LIMIT 35.0      // Ngưỡng cháy (độ C)
#define DHT_HUM_LIMIT 85.0       // Ngưỡng ẩm mốc (%)
#define FIRE_RATE_LIMIT 1.5      // Tốc độ tăng nhiệt bất thường (độ C / 2 giây)
#define DHT_SAFE_TEMP 10.0  // Nhiệt độ an toàn để hệ thống hoạt động bình thường trở lại
#define PARK_SAMPLE_COUNT 30
#define PARK_THRESHOLD 24
// --- BIẾN TOÀN CỤC ---
LCD_Typedef myLCD;
Servo_Typedef myServo[2]; // Servo[0]: Cổng vào, Servo[1]: Cổng ra
QueueHandle_t xLCDQueue;
QueueHandle_t xGateQueue ;
int current_slots = MAX_SLOTS;
float last_temp = -1.0;
float temp, hum;
bool is_emergency = false; // Cờ báo cháy khẩn cấp
float last_display_temp = -1.0;
float last_display_hum = -1.0;
typedef struct {
    char line1[17];
    char line2[17];
} lcd_msg_t;
typedef struct {
    int gate_id; // 1 hoặc 2
    int action;  // 0: close, 1: open
    int timed;   // 1: auto close after GATE_OPEN_TIME_MS, 0: manual
} gate_command_t;
int last_states[3] = {-1, -1, -1};
int PinPark[3] = {14,26,27} ;


// --- CALLBACK MQTT (TRUNG TÂM XỬ LÝ LỆNH TỪ SERVER) ---
void on_mqtt_data(const char *topic, const char *data, int data_len) {
    ESP_LOGI("MAIN", "Nh?n d? li?u t? Topic: %s", topic);
    ESP_LOGI("MAIN", "N?i dung: %.*s (len=%d)", data_len, data, data_len);
    if (data == NULL || data_len == 0) return;
    if (strcmp(topic, "/ptit/servo/control") == 0) {
        gate_command_t cmd;
        if (strstr(data, "1") != NULL) {
            cmd.gate_id = 1;
            cmd.action = 1; // open
            cmd.timed = 1; // auto close
            xQueueSend(xGateQueue, &cmd, 0);
            ESP_LOGI("MQTT", "Da day lenh MO CONG 1 vao Queue");
        } 
        else if (strstr(data, "2") != NULL) {
            cmd.gate_id = 2;
            cmd.action = 1; // open
            cmd.timed = 1; // auto close
            xQueueSend(xGateQueue, &cmd, 0);
            ESP_LOGI("MQTT", "Da day lenh MO CONG 2 vao Queue");
        }
    }

    // Xử lý topic khẩn cấp
    if (strcmp(topic, "/ptit/servo/emergency") == 0) {
        gate_command_t cmd;
        if (strcmp(data, "O1") == 0) {
            cmd.gate_id = 1;
            cmd.action = 1; // open cổng vào
            cmd.timed = 0; // manual
            xQueueSend(xGateQueue, &cmd, 0);
            ESP_LOGI("MQTT", "Emergency: Mo cong vao (O1)");
        } else if (strcmp(data, "C1") == 0) {
            cmd.gate_id = 1;
            cmd.action = 0; // close cổng vào
            cmd.timed = 0; // manual
            xQueueSend(xGateQueue, &cmd, 0);
            ESP_LOGI("MQTT", "Emergency: Dong cong vao (C1)");
        } else if (strcmp(data, "O2") == 0) {
            cmd.gate_id = 2;
            cmd.action = 1; // open cổng ra
            cmd.timed = 0; // manual
            xQueueSend(xGateQueue, &cmd, 0);
            ESP_LOGI("MQTT", "Emergency: Mo cong ra (O2)");
        } else if (strcmp(data, "C2") == 0) {
            cmd.gate_id = 2;
            cmd.action = 0; // close cổng ra
            cmd.timed = 0; // manual
            xQueueSend(xGateQueue, &cmd, 0);
            ESP_LOGI("MQTT", "Emergency: Dong cong ra (C2)");
        }
    }
}

// --- HÀM HỖ TRỢ ---
void LCD_Queue_Send(const char* l1, const char* l2) {
    lcd_msg_t msg;
    memset(&msg, 0, sizeof(lcd_msg_t));
    if (l1) strncpy(msg.line1, l1, 16);
    if (l2) strncpy(msg.line2, l2, 16);
    xQueueSend(xLCDQueue, &msg, 0);
}

void Show_Default_Display() {
    char buf[17];
    char buf1[17];
    if (current_slots <= 0) {
        LCD_Queue_Send("full", "het cho!!!");
        ESP_LOGI("DISPLAY", "Sent: full|het cho!!!");
    } else {
        snprintf(buf, sizeof(buf), "s1:%d s2:%d s3:%d", last_states[0], last_states[1], last_states[2]);
        snprintf(buf1, sizeof(buf1), "%.1fC, %.1f%%", temp, hum);
        LCD_Queue_Send(buf, buf1);
        ESP_LOGI("DISPLAY", "Sent: %s|%s", buf,buf1);
    }
}


// Task 1: Quản lý hiển thị LCD
void LCD_Manager_Task(void *pvParameters) {
    ESP_LOGE("LCD_TASK","bat dau") ;
    lcd_msg_t received;
    while (1) {
        if (xQueueReceive(xLCDQueue, &received, portMAX_DELAY)) {
            ESP_LOGI("LCD_TASK", "Received: L1='%s' L2='%s'", received.line1, received.line2);//
            lcd_clear(&myLCD);
            lcd_set_cursor(&myLCD, 0, 0);
            lcd_send_string(&myLCD, received.line1);
            lcd_set_cursor(&myLCD, 1, 0);
            lcd_send_string(&myLCD, received.line2);
            if (strstr(received.line1, "phi")) {
                vTaskDelay(pdMS_TO_TICKS(2000)); 
            }
        }
    }
}

// Task 2: Quẹt thẻ (Chỉ gửi UID, không tự mở cổng)
void RFID_Task(void *pvParameters) {
    ESP_LOGE("RFID_TASK","bat dau") ;
    unsigned char uid[5];
    char uid_str[15];
    while (1) {
        if (is_emergency) {
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue; 
        }
        if (MFRC522_Check(uid) == MI_OK) {
            sprintf(uid_str, "%02X%02X%02X%02X%02X", uid[0], uid[1], uid[2], uid[3], uid[4]);
            my_mqtt_publish("/ptit/parking/check", uid_str, 0); 
            vTaskDelay(pdMS_TO_TICKS(2500));
            Show_Default_Display();
        }
        vTaskDelay(pdMS_TO_TICKS(150));
    }
}

void Parking_Sensor_Task(void *pvParameters) {
    ESP_LOGE("PARKING_TASK","bat dau") ;
    
    // Bộ đệm vòng (circular buffer) lưu 30 mẫu gần nhất cho 3 cảm biến
    int history[3][PARK_SAMPLE_COUNT] = {0};
    int history_idx = 0;
    int sum[3] = {0};
    bool is_first_run = true;

    while (1) {
        int occupied = 0;
        bool changed = false;
        for (int i = 0; i < 3; i++) {
            int state = gpio_get_level(PinPark[i]); 
            if (is_first_run) {
                for(int j = 0; j < PARK_SAMPLE_COUNT; j++) history[i][j] = state;
                sum[i] = state * PARK_SAMPLE_COUNT;
            } else {
                sum[i] -= history[i][history_idx];
                history[i][history_idx] = state;
                sum[i] += state;
            }
            
            int confirmed_state = last_states[i];
            if (sum[i] >= PARK_THRESHOLD) confirmed_state = 1;
            else if (sum[i] <= (PARK_SAMPLE_COUNT - PARK_THRESHOLD)) confirmed_state = 0;
            else if (confirmed_state == -1) confirmed_state = state; // Xử lý lần đọc đầu tiên
            
            if (confirmed_state == 1) occupied++;
            
            if (confirmed_state != last_states[i]) {
                char topic[32], val[2];
                sprintf(topic, "/state/park/%d", i + 1);
                sprintf(val, "%d", confirmed_state);
                my_mqtt_publish(topic, val, 1); 
                last_states[i] = confirmed_state;
                changed = true;
            }
        }
        current_slots = MAX_SLOTS - occupied;
        if (changed) Show_Default_Display();
        
        is_first_run = false;
        history_idx = (history_idx + 1) % PARK_SAMPLE_COUNT; 
        vTaskDelay(pdMS_TO_TICKS(100)); 
    }
}

void Gate_Control_Task(void *pvParameters) {
    ESP_LOGE("GATE_TASK","bat dau") ;
    gate_command_t cmd;
    while (1) {
        if (xQueueReceive(xGateQueue, &cmd, portMAX_DELAY)) {
            int idx = cmd.gate_id - 1; // Cổng 1 -> index 0, Cổng 2 -> index 1
            if (cmd.action == 1) {
                // Mở cổng
                servo_write_angle_slow(&myServo[idx], 90, 20);
                ESP_LOGI("GATE_TASK", "Da mo cong %d", cmd.gate_id);
                if (cmd.timed) {
                    // Đợi thời gian mở cổng rồi tự động đóng
                    vTaskDelay(pdMS_TO_TICKS(GATE_OPEN_TIME_MS));
                    servo_write_angle_slow(&myServo[idx], 0, 20);
                    ESP_LOGI("GATE_TASK", "Da tu dong dong cong %d sau %d ms", cmd.gate_id, GATE_OPEN_TIME_MS);
                }
            } else if (cmd.action == 0) {
                servo_write_angle_slow(&myServo[idx], 0, 20);
                ESP_LOGI("GATE_TASK", "Da dong cong %d", cmd.gate_id);
            }
            Show_Default_Display();
        }
    }
}

void Environment_Monitoring_Task(void *pvParameters) {
    ESP_LOGI("ENVIRONMENT_TASK", "Bat dau");
    char log_buf[64];
    while (1) {
        if (read_dht11(&temp, &hum) == ESP_OK) {
            float temp_rate = (last_temp > 0) ? (temp - last_temp) : 0;
            sprintf(log_buf, "%.1fC, %.1f%%", temp, hum);
            my_mqtt_publish("/ptit/parking/environment", log_buf, 1);


            // --- KỊCH BẢN 1: PHÁT HIỆN NHIỆT ĐỘ TĂNG BẤT THƯỜNG ---
            if (temp >= DHT_TEMP_LIMIT) {
                if (!is_emergency) { 
                    is_emergency = true;
                    gate_command_t cmd_open1 = {.gate_id=1,.action = 1, .timed = 0};
                    gate_command_t cmd_open2 = {.gate_id=2, .action =1, .timed = 0} ;
                    xQueueSend(xGateQueue, &cmd_open1, 0); // Mở cổng...
                    xQueueSend(xGateQueue, &cmd_open2, 0);
                    LCD_Queue_Send("!!! CHAY !!!", "MO CONG THOAT");
                    my_mqtt_publish("/ptit/parking/alarm", "FIRE", 1);
                }
            } 
            // --- KỊCH BẢN 2: NHIỆT ĐỘ XUỐNG MỨC AN TOÀN AN TOÀN ---
            else if (is_emergency && temp < DHT_TEMP_LIMIT) {
                is_emergency = false;
                 gate_command_t cmd_open3 = {.gate_id=1,.action = 0, .timed = 0};
                 gate_command_t cmd_open4 = {.gate_id=2, .action =0, .timed = 0} ;
                xQueueSend(xGateQueue, &cmd_open3, 0); // Mở cổng...
                xQueueSend(xGateQueue, &cmd_open4, 0);
                last_display_temp = -1.0; 
                Show_Default_Display();
            }
            if (!is_emergency) {
                // Kiểm tra xem giá trị có thay đổi không (so sánh số thực)
                // Dùng chênh lệch > 0.1 để tránh nhiễu nhảy số nhỏ
                if (temp != last_display_temp || hum != last_display_hum) {
                    Show_Default_Display();
                    
                    // Cập nhật lại giá trị hiển thị gần nhất
                    last_display_temp = temp;
                    last_display_hum = hum;
                    ESP_LOGI("DISPLAY", "Gia tri thay doi -> Cap nhat LCD");
                }
            }

            last_temp = temp;
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
void app_main(void) {
    vTaskDelay(pdMS_TO_TICKS(2000));
    flash_init();
    wifi_init_sta("iPhone", "12345678");
    vTaskDelay(pdMS_TO_TICKS(5000)); 
    my_mqtt_init("mqtt://broker.emqx.io", on_mqtt_data);
    xLCDQueue = xQueueCreate(10, sizeof(lcd_msg_t));
    xGateQueue = xQueueCreate(5, sizeof(gate_command_t));

    // 4. Khởi tạo ngoại vi phần cứng (I2C, SPI, GPIO, Servo)
    // Cấu hình GPIO cảm biến
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL<<14) | (1ULL<<27) | (1ULL<<26),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE
    };
    gpio_config(&io_conf);
    gpio_reset_pin(DHT11_PIN);

    // Khởi tạo các thiết bị
    i2c_master_init(&myLCD, 21, 22, I2C_NUM_0, 50000, 0x27);
    lcd_init(&myLCD);
    spi_init(18, 23, 19); 
    MFRC522_Init(17, 5);  
    servo_init(&myServo[0], GPIO_NUM_2, LEDC_CHANNEL_0); // Cổng vào
    servo_init(&myServo[1], GPIO_NUM_4, LEDC_CHANNEL_1); // Cổng ra

    xTaskCreatePinnedToCore(LCD_Manager_Task, "LCD_Task", 3072, NULL, 4, NULL, 1);
    xTaskCreatePinnedToCore(Gate_Control_Task, "Gate_Task", 4096, NULL, 3, NULL, 1);
    xTaskCreatePinnedToCore(RFID_Task, "RFID_Task", 4096, NULL, 3, NULL, 1);
    xTaskCreatePinnedToCore(Environment_Monitoring_Task, "Env_Task", 4096, NULL, 2, NULL, 1);
    xTaskCreatePinnedToCore(Parking_Sensor_Task, "Park_Task", 3072, NULL, 1, NULL, 1);
    Show_Default_Display();
}