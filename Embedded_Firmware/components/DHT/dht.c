#include "dht.h"
#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "rom/ets_sys.h"

// Sử dụng portMUX để khóa ngắt trên ESP32
static portMUX_TYPE dht_mux = portMUX_INITIALIZER_UNLOCKED;

int get_signal_level(int us_time, int level) {
    int cnt = 0;
    while (gpio_get_level(DHT11_PIN) == level) {
        if (cnt >= us_time) return -1; 
        cnt++;
        ets_delay_us(1);
    }
    return cnt;
}

esp_err_t read_dht11(float *temp, float *hum) {
    uint8_t data[5] = {0, 0, 0, 0, 0};

    // 1. MCU gửi tín hiệu Start
    gpio_set_direction(DHT11_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(DHT11_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(20)); 
    gpio_set_level(DHT11_PIN, 1);
    ets_delay_us(30); 
    gpio_set_direction(DHT11_PIN, GPIO_MODE_INPUT);

    // 2. Khóa ngắt để đọc 40 bit chính xác
    portENTER_CRITICAL(&dht_mux);

    // Check Response từ DHT11
    if (get_signal_level(100, 0) < 0 || get_signal_level(100, 1) < 0) {
        portEXIT_CRITICAL(&dht_mux);
        return ESP_ERR_TIMEOUT;
    }

    for (int i = 0; i < 40; i++) {
        if (get_signal_level(100, 0) < 0) {
            portEXIT_CRITICAL(&dht_mux);
            return ESP_ERR_TIMEOUT;
        }
        
        int high_level_time = get_signal_level(100, 1);
        if (high_level_time < 0) {
            portEXIT_CRITICAL(&dht_mux);
            return ESP_ERR_TIMEOUT;
        }

        data[i / 8] <<= 1;
        if (high_level_time > 40) { 
            data[i / 8] |= 1;
        }
    }
    // 3. Xong việc nhạy cảm thì mở khóa ngắt ngay
    portEXIT_CRITICAL(&dht_mux);
    // 4. Kiểm tra Checksum
    if (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {
        *hum = (float)data[0];
        *temp = (float)data[2];
        return ESP_OK;
    } else {
        return ESP_ERR_INVALID_CRC;
    }
}