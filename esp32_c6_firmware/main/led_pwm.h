#ifndef LED_PWM_H
#define LED_PWM_H

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

// GPIO Definitions for ESP32-C6-Zero
#define LEDC_GPIO_R    (1)
#define LEDC_GPIO_G    (2)
#define LEDC_GPIO_B    (3)
#define LEDC_GPIO_W    (4)

typedef struct {
    uint8_t r, g, b, w;
} rgbw_state_t;

// Global Shared State
extern rgbw_state_t target_state;
extern SemaphoreHandle_t ble_mutex;

// Function Prototypes
void led_pwm_init(void);
void led_task(void *pvParameters);

#endif
