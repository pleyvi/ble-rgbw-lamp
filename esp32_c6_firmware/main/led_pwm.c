#include "led_pwm.h"
#include "driver/ledc.h"
#include "esp_log.h"

static const char *TAG = "LED_PWM";

// Initialize the shared state variables
rgbw_state_t target_state = {0, 0, 0, 0};
SemaphoreHandle_t ble_mutex = NULL;

void led_pwm_init(void) {
    // 1. Configure the PWM Timer
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,
        .timer_num        = LEDC_TIMER_0,
        .duty_resolution  = LEDC_TIMER_8_BIT,
        .freq_hz          = 5000,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);

    // 2. Configure the 4 channels
    int gpios[] = {LEDC_GPIO_R, LEDC_GPIO_G, LEDC_GPIO_B, LEDC_GPIO_W};
    for (int i = 0; i < 4; i++) {
        ledc_channel_config_t ledc_channel = {
            .speed_mode     = LEDC_LOW_SPEED_MODE,
            .channel        = i,
            .timer_sel      = LEDC_TIMER_0,
            .intr_type      = LEDC_INTR_DISABLE,
            .gpio_num       = gpios[i],
            .duty           = 0,
            .hpoint         = 0
        };
        ledc_channel_config(&ledc_channel);
    }
    
    // Create the mutex for thread-safe state updates
    ble_mutex = xSemaphoreCreateMutex();
    ESP_LOGI(TAG, "PWM Hardware and Mutex initialized.");
}

void led_task(void *pvParameters) {
    while (1) {
        if (ble_mutex != NULL && xSemaphoreTake(ble_mutex, portMAX_DELAY) == pdTRUE) {
            uint8_t r = target_state.r;
            uint8_t g = target_state.g;
            uint8_t b = target_state.b;
            uint8_t w = target_state.w;
            xSemaphoreGive(ble_mutex);

            ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, r);
            ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, g);
            ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2, b);
            ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_3, w);

            for(int i=0; i<4; i++) ledc_update_duty(LEDC_LOW_SPEED_MODE, i);
        }
        vTaskDelay(pdMS_TO_TICKS(20)); // 50Hz update rate
    }
}
