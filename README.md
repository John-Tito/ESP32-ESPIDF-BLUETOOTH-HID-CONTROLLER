# ESP32-ESPIDF-BLUETOOTH-HID-CONTROLLER

#### 介绍
基于ESPIDF HID例程修改而来，使用经典蓝牙和Xbox ONE 手柄连接并进行键值解析

ESP-IDF BT/BLE HID Host Demo Connect to XBOX ONE S Controller

使用手柄左扳机和右扳机键来控制PWM占空比的例子

An example use left triger and right triger of the controller to change the dutu of pwm
#### 软件架构

框架/framework:[platform-espressif32(Releases 3.5.0)](https://github.com/platformio/platform-espressif32)

组件/components:

 - [XboxControllerNotificationParser](https://github.com/asukiaaa/arduino-XboxControllerNotificationParser)

 - [controlerHandler](./components/controlerHandler/controlerHandler.cpp)

 - [esp_hid_gap](./components/esp_hid_gap/esp_hid_gap.c)

    参考 framework-espidf\examples\bluetooth\esp_hid_host
