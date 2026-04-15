#include <stdio.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "led_pwm.h"

// Forward declaration of our GATT helper
extern int gatt_svr_init(void);

static const char *TAG = "BLE_MAIN";
static const char *DEVICE_NAME = "ESP32-C6-RGBW";

void on_stack_sync(void) {
    struct ble_hs_adv_fields fields = {0};
    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
    fields.name = (uint8_t *)DEVICE_NAME;
    fields.name_len = strlen(DEVICE_NAME);
    fields.name_is_complete = 1;

    ble_gap_adv_set_fields(&fields);

    struct ble_gap_adv_params adv_params = {0};
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;

    ble_gap_adv_start(BLE_OWN_ADDR_PUBLIC, NULL, BLE_HS_FOREVER, &adv_params, NULL, NULL);
    ESP_LOGI(TAG, "Advertising Started");
}

void host_task(void *param) {
    nimble_port_run();
    nimble_port_freertos_deinit();
}

void app_main(void) {
    ESP_ERROR_CHECK(nvs_flash_init());

    nimble_port_init();
    ble_svc_gap_device_name_set(DEVICE_NAME);
    led_pwm_init();
    xTaskCreate(led_task, "led_task", 4096, NULL, 5, NULL);
    // Initialize our separated GATT server
    int rc = gatt_svr_init();
    assert(rc == 0);

    ble_hs_cfg.sync_cb = on_stack_sync;
    nimble_port_freertos_init(host_task);
}
