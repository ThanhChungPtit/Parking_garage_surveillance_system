#ifndef __SERVO_H
#define __SERVO_H

#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

typedef struct {
    int pin;
    ledc_channel_t channel;
    ledc_timer_t timer;
    uint32_t min_degree;
    uint32_t max_degree;
    uint32_t min_pulse_us;
    uint32_t max_pulse_us;
    uint32_t current_angle;
} Servo_Typedef;

// Khởi tạo PWM cho Servo
void servo_init(Servo_Typedef *servo, int pin, ledc_channel_t channel);

// Điều khiển góc quay (0 - 180)
void servo_write_angle(Servo_Typedef *servo, uint32_t angle);
void servo_write_angle_slow(Servo_Typedef *servo, uint32_t target_angle, uint32_t delay_ms) ;

#endif