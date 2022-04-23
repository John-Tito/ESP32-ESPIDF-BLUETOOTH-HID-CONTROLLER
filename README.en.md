# ESP32-ESPIDF-BLUETOOTH-HID-CONTROLLER

#### Description
ESP-IDF BT/BLE HID Host Demo Connect to XBOX ONE S Controller

An example use left triger and right triger of the controller to change the dutu of pwm

#### Software Architecture
framework:

[platform-espressif32(Releases 3.5.0)](https://github.com/platformio/platform-espressif32)

components:

 - [XboxControllerNotificationParser](https://github.com/asukiaaa/arduino-XboxControllerNotificationParser)

 - [controlerHandler](./components/controlerHandler/controlerHandler.cpp)

 - [esp_hid_gap](./components/esp_hid_gap/esp_hid_gap.c)

    refer to framework-espidf\examples\bluetooth\esp_hid_host
