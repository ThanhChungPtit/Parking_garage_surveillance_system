#include "servo.h"

#define SERVO_FREQ      50          // 50Hz (20ms)
#define SERVO_RES       LEDC_TIMER_13_BIT 
#define SERVO_DUTY_MAX  8191        // 2^13 - 1

void servo_init(Servo_Typedef *servo, int pin, ledc_channel_t channel) {
    servo->pin = pin;
    servo->channel = channel;
    servo->timer = LEDC_TIMER_0;
    servo->min_degree = 0;
    servo->max_degree = 180;
    servo->min_pulse_us = 500;  // 0.5ms
    servo->max_pulse_us = 2500; // 2.5ms

    // Cấu hình Timer
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,
        .timer_num        = servo->timer,
        .duty_resolution  = SERVO_RES,
        .freq_hz          = SERVO_FREQ,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);

    // Cấu hình Channel
    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_LOW_SPEED_MODE,
        .channel        = servo->channel,
        .timer_sel      = servo->timer,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = servo->pin,
        .duty           = 0,
        .hpoint         = 0
    };
    ledc_channel_config(&ledc_channel);
    servo->current_angle = 0; 
    servo_write_angle(servo, 0); // Đưa servo về 0 độ khi bắt đầu
}

void servo_write_angle(Servo_Typedef *servo, uint32_t angle) {
    if (angle > servo->max_degree) angle = servo->max_degree;

    // Tính toán độ rộng xung (us) từ góc quay
    uint32_t pulse_us = servo->min_pulse_us + 
                       (angle * (servo->max_pulse_us - servo->min_pulse_us) / servo->max_degree);
    
    // Chuyển đổi us sang giá trị Duty Cycle (13-bit)
    // Công thức: duty = (pulse_us / chu_ky_us) * (2^resolution - 1)
    // Chu kỳ 50Hz = 20000us
    uint32_t duty = (pulse_us * SERVO_DUTY_MAX) / 20000;

    ledc_set_duty(LEDC_LOW_SPEED_MODE, servo->channel, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, servo->channel);
}

void servo_write_angle_slow(Servo_Typedef *servo, uint32_t target_angle, uint32_t delay_ms) {
    if (target_angle > servo->max_degree) target_angle = servo->max_degree;

    // Xác định hướng quay (tăng hay giảm)
    int step = (target_angle > servo->current_angle) ? 1 : -1;

    // Chạy vòng lặp từ góc hiện tại đến góc đích
    while (servo->current_angle != target_angle) {
        servo->current_angle += step;
        servo_write_angle(servo, servo->current_angle);
        vTaskDelay(pdMS_TO_TICKS(delay_ms));
    }
}

