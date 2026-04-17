#ifndef LED_PWM_H
#define LED_PWM_H

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#define LEDC_GPIO_R    (1)
#define LEDC_GPIO_G    (2)
#define LEDC_GPIO_B    (3)
#define LEDC_GPIO_W    (4)

typedef struct {
    uint8_t r, g, b, w;
    uint8_t smoothing_enabled; // 1 = Fade, 0 = Instant snap
} rgbw_state_t;

extern rgbw_state_t target_state;
extern SemaphoreHandle_t ble_mutex;

void led_pwm_init(void);
void led_task(void *pvParameters);

#endif