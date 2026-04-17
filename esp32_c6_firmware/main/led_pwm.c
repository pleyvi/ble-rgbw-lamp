#include "led_pwm.h"
#include "driver/ledc.h"
#include "esp_log.h"

static const char *TAG = "LED_PWM";

// Initialize with smoothing ON by default (1)
rgbw_state_t target_state = {0, 0, 0, 0, 1}; 
SemaphoreHandle_t ble_mutex = NULL;

void led_pwm_init(void) {
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,
        .timer_num        = LEDC_TIMER_0,
        .duty_resolution  = LEDC_TIMER_8_BIT,
        .freq_hz          = 5000,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);

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
    
    ble_mutex = xSemaphoreCreateMutex();
    ESP_LOGI(TAG, "PWM Hardware initialized. Smoothing Enabled by default.");
}

// Helper function for linear interpolation
static int16_t move_towards(int16_t current, int16_t target, int16_t step) {
    if (current < target) {
        return (current + step > target) ? target : current + step;
    } else if (current > target) {
        return (current - step < target) ? target : current - step;
    }
    return current;
}

void led_task(void *pvParameters) {
    // Track the physical state of the LEDs locally
    int16_t cur_r = 0, cur_g = 0, cur_b = 0, cur_w = 0;
    
    // Fade speed: 5 units per 20ms tick (~1 second for a full 0-255 sweep)
    const int16_t FADE_STEP = 5; 

    while (1) {
        if (ble_mutex != NULL && xSemaphoreTake(ble_mutex, portMAX_DELAY) == pdTRUE) {
            
            // Read targets and settings from the shared state
            uint8_t tgt_r = target_state.r;
            uint8_t tgt_g = target_state.g;
            uint8_t tgt_b = target_state.b;
            uint8_t tgt_w = target_state.w;
            uint8_t smooth = target_state.smoothing_enabled;
            xSemaphoreGive(ble_mutex);

            if (smooth) {
                cur_r = move_towards(cur_r, tgt_r, FADE_STEP);
                cur_g = move_towards(cur_g, tgt_g, FADE_STEP);
                cur_b = move_towards(cur_b, tgt_b, FADE_STEP);
                cur_w = move_towards(cur_w, tgt_w, FADE_STEP);
            } else {
                cur_r = tgt_r;
                cur_g = tgt_g;
                cur_b = tgt_b;
                cur_w = tgt_w;
            }

            ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, cur_r);
            ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, cur_g);
            ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2, cur_b);
            ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_3, cur_w);

            for(int i=0; i<4; i++) ledc_update_duty(LEDC_LOW_SPEED_MODE, i);
        }
        
        vTaskDelay(pdMS_TO_TICKS(20)); // 50Hz update loop
    }
}
