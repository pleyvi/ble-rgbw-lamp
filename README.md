# BLE RGBW Lamp Controller
Reference implementation for a Bluetooth Low Energy (BLE) RGBW LED controller. This project bridges a bare-metal ESP-IDF firmware implementation on an ESP32-C6 with a Flutter-based Android remote control.

The architecture strictly separates the radio stack from the hardware PWM timers using FreeRTOS tasks and thread-safe memory sharing.

## Hardware Stack
MCU: ESP32-C6-Zero

Drivers: 4x HW-532 MOSFET modules (N-channel, low-side switching)

Power Topology: Dual USB supply for isolation.

5V standard USB for the ESP32 logic.

12V PD trigger board for the LED load.

Common ground tied across all MOSFETs and the ESP32 to prevent floating gates.

Load: 12V Common-Anode RGBW LED strip.

Basic wiring diagram is in the hardware/ directory.

## Firmware Architecture (ESP-IDF)
Built with ESP-IDF v5.5. The firmware avoids putting hardware control inside radio callbacks. It is split into three main concerns:

ble_led_controller.c: Main entry point. Handles NVS initialization, orchestrates boot, and spawns the FreeRTOS tasks.

gatt_svr.c: Implements the NimBLE host stack. Runs on its own high-priority task. Exposes a GATT server using 16-bit UUIDs (Service: 0x1812, Characteristic: 0x2A3D) to avoid endianness parsing issues with mobile clients. Incoming 4-byte payloads (R, G, B, W) are locked via Mutex and pushed to a shared state struct.

led_pwm.c: Hardware driver. Configures the ESP32 LEDC peripheral for 5kHz, 8-bit resolution hardware PWM. Runs a dedicated 50Hz FreeRTOS task that reads the shared state (via Mutex) and updates the DMA buffers, ensuring zero flicker regardless of BLE radio traffic.

## Mobile Client (Flutter)
A minimal Android client built with Flutter and flutter_blue_plus.

Bypasses heavy state management in favor of a direct event-driven BLE connection.

Implements fire-and-forget payload writing (withoutResponse: true) to ensure the slider UI remains fluid without choking the BLE stack with acknowledgment packets.

Handles the License.free requirement enforced by newer versions of the flutter_blue_plus package during the connection handshake.

## Build and Flash
Firmware:
Requires ESP-IDF v5.5+.

`cd esp32_c6_firmware; 
idf.py set-target esp32c6; 
idf.py build flash monitor`

## Mobile App:
Requires the Flutter SDK. Target a physical Android device, as the Android emulator does not support BLE passthrough.

`cd flutter_android_remote_control_app; 
flutter run`

