#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "host/ble_hs.h"
#include "services/gatt/ble_svc_gatt.h"
#include "led_pwm.h"

// Shared state structure
//definition moved
static const char *TAG = "GATT_SVR";

// Use 16-bit UUIDs for simplicity
#define SVC_UUID 0x1812 
#define CHR_UUID 0x2A3D 

uint16_t led_handle;

static int gatt_svr_access_led(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg) {
    if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
        uint8_t *data = ctxt->om->om_data;
        uint16_t len = OS_MBUF_PKTLEN(ctxt->om);
        
        if (len == 4 && ble_mutex != NULL) {
            // Lock the mutex before writing to shared state
            if (xSemaphoreTake(ble_mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
                target_state.r = data[0];
                target_state.g = data[1];
                target_state.b = data[2];
                target_state.w = data[3];
                xSemaphoreGive(ble_mutex);
                
                ESP_LOGI(TAG, "State Updated: R:%d G:%d B:%d W:%d", 
                         target_state.r, target_state.g, target_state.b, target_state.w);
            }
        }
        return 0;
    }
    return BLE_ATT_ERR_UNLIKELY;
}

static const struct ble_gatt_svc_def gatt_svr_svcs[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(SVC_UUID),
        .characteristics = (struct ble_gatt_chr_def[]) { {
            .uuid = BLE_UUID16_DECLARE(CHR_UUID),
            .access_cb = gatt_svr_access_led,
            .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_NO_RSP,
            .val_handle = &led_handle,
        }, {
            0,
        } },
    },
    {0},
};

int gatt_svr_init(void) {
    int rc;
    ble_svc_gatt_init();
    rc = ble_gatts_count_cfg(gatt_svr_svcs);
    if (rc != 0) return rc;
    rc = ble_gatts_add_svcs(gatt_svr_svcs);
    if (rc != 0) return rc;
    return 0;
}
