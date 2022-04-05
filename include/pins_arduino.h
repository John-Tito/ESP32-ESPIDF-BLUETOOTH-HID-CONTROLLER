#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include <stdint.h>

#define EXTERNAL_NUM_INTERRUPTS 16
#define NUM_DIGITAL_PINS 40
#define NUM_ANALOG_INPUTS 16

#define analogInputToDigitalPin(p) (((p) < 20) ? (esp32_adc2gpio[(p)]) : -1)
#define digitalPinToInterrupt(p) (((p) < 40) ? (p) : -1)
#define digitalPinHasPWM(p) (p < 34)

static const uint8_t TX = 1;
static const uint8_t RX = 3;

static const uint8_t LED_BUILTIN = 5;
#define BUILTIN_LED LED_BUILTIN // backward compatibility

static const uint8_t SDA = 21;
static const uint8_t SCL = 22;

static const uint8_t SS = 5;
static const uint8_t MOSI = 23;
static const uint8_t MISO = 19;
static const uint8_t SCK = 18;

static const uint8_t VSPI_MISO = 19;
static const uint8_t VSPI_MOSI = 23;
static const uint8_t VSPI_SCLK = 18;
static const uint8_t VSPI_SS = 5;

static const uint8_t HSPI_MISO = 12;
static const uint8_t HSPI_MOSI = 13;
static const uint8_t HSPI_SCLK = 14;
static const uint8_t HSPI_SS = 15;

static const uint8_t SENSOR_VP = 36;
static const uint8_t SENSOR_VN = 39;

static const uint8_t A0 = 36;
static const uint8_t A3 = 39;
static const uint8_t A4 = 32;
static const uint8_t A5 = 33;
static const uint8_t A6 = 34;
static const uint8_t A7 = 35;
static const uint8_t A10 = 4;
static const uint8_t A11 = 0;
static const uint8_t A12 = 2;
static const uint8_t A13 = 15;
static const uint8_t A14 = 13;
static const uint8_t A15 = 12;
static const uint8_t A16 = 14;
static const uint8_t A17 = 27;
static const uint8_t A18 = 25;
static const uint8_t A19 = 26;

static const uint8_t T0 = 4;
static const uint8_t T1 = 0;
static const uint8_t T2 = 2;
static const uint8_t T3 = 15;
static const uint8_t T4 = 13;
static const uint8_t T5 = 12;
static const uint8_t T6 = 14;
static const uint8_t T7 = 27;
static const uint8_t T8 = 33;
static const uint8_t T9 = 32;

static const uint8_t DAC1 = 25;
static const uint8_t DAC2 = 26;

//UART
static const uint8_t U0CTS = 19;
static const uint8_t U0RTS = 22;
static const uint8_t U0RXD = 3;
static const uint8_t U0TXD = 1;
//UART
static const uint8_t U1CTSS = 6;
static const uint8_t U1RTSS = 11;
static const uint8_t U1RXDS = 9;
static const uint8_t U1TXDS = 10;
//UART
static const uint8_t U2CTS = 8;
static const uint8_t U2RTS = 7;
//ADC
static const uint8_t ADC1_CH0 = 36;
static const uint8_t ADC1_CH3 = 39;
static const uint8_t ADC1_CH4 = 32;
static const uint8_t ADC1_CH5 = 33;
static const uint8_t ADC1_CH6 = 34;
static const uint8_t ADC1_CH7 = 35;
//ADC
static const uint8_t ADC2_CH0 = 4;
static const uint8_t ADC2_CH1 = 0;
static const uint8_t ADC2_CH2 = 2;
static const uint8_t ADC2_CH3 = 15;
static const uint8_t ADC2_CH4 = 13;
static const uint8_t ADC2_CH5 = 12;
static const uint8_t ADC2_CH6 = 14;
static const uint8_t ADC2_CH7 = 27;
static const uint8_t ADC2_CH8 = 25;
static const uint8_t ADC2_CH9 = 26;
//SPI
static const uint8_t VSPIWP = 22;
static const uint8_t VSPIQ = 19;
static const uint8_t VSPIHD = 21;
static const uint8_t VSPID = 23;
static const uint8_t VSPICS0 = 5;
static const uint8_t VSPICLK = 18;
//SPI
static const uint8_t SPIWP = 10;
static const uint8_t SPIQ = 7;
static const uint8_t SPIHD = 9;
static const uint8_t SPID = 8;
static const uint8_t SPICS0 = 11;
static const uint8_t SPICLK = 6;
//SPI
static const uint8_t HSPIWP = 2;
static const uint8_t HSPIQ = 12;
static const uint8_t HSPIHD = 4;
static const uint8_t HSPID = 13;
static const uint8_t HSPICS0 = 15;
static const uint8_t HSPICLK = 14;
//RTC
static const uint8_t RTC_17 = 27;
static const uint8_t RTC_16 = 14;
static const uint8_t RTC_15 = 12;
static const uint8_t RTC_14 = 13;
static const uint8_t RTC_13 = 15;
static const uint8_t RTC_12 = 2;
static const uint8_t RTC_11 = 0;
static const uint8_t RTC_10 = 4;
static const uint8_t RTC_09 = 32;
static const uint8_t RTC_08 = 33;
static const uint8_t RTC_07 = 26;
static const uint8_t RTC_06 = 25;
static const uint8_t RTC_05 = 35;
static const uint8_t RTC_04 = 34;
static const uint8_t RTC_03 = 39;
static const uint8_t RTC_00 = 36;
//TOUCH
static const uint8_t TOUCH9 = 32;
static const uint8_t TOUCH8 = 33;
static const uint8_t TOUCH7 = 27;
static const uint8_t TOUCH6 = 14;
static const uint8_t TOUCH5 = 12;
static const uint8_t TOUCH4 = 13;
static const uint8_t TOUCH3 = 15;
static const uint8_t TOUCH2 = 2;
static const uint8_t TOUCH1 = 0;
static const uint8_t TOUCH0 = 4;
//EMAC
static const uint8_t EMAC_TXD3 = 12;
static const uint8_t EMAC_TXD2 = 14;
static const uint8_t EMAC_TXD1 = 22;
static const uint8_t EMAC_TXD0 = 19;
static const uint8_t EMAC_TX_ER = 4;
static const uint8_t EMAC_TX_EN = 21;
static const uint8_t EMAC_TX_CLK = 0;
static const uint8_t EMAC_RXD3 = 15;
static const uint8_t EMAC_RXD2 = 1;
static const uint8_t EMAC_RXD1 = 26;
static const uint8_t EMAC_RXD0 = 25;
static const uint8_t EMAC_RX_ERS = 13;
static const uint8_t EMAC_RX_DV = 27;
static const uint8_t EMAC_RX_CLK = 5;
//HS
static const uint8_t HS2_DATA3 = 13;
static const uint8_t HS2_DATA2 = 12;
static const uint8_t HS2_DATA1 = 4;
static const uint8_t HS2_DATA0 = 2;
static const uint8_t HS2_CMD = 15;
static const uint8_t HS2_CLK = 14;
//HS
static const uint8_t HS1_STROBE = 23;
static const uint8_t HS1_DATA7 = 18;
static const uint8_t HS1_DATA6 = 5;
static const uint8_t HS1_DATA3 = 10;
static const uint8_t HS1_DATA2 = 9;
static const uint8_t HS1_DATA1 = 8;
static const uint8_t HS1_DATA0 = 7;
static const uint8_t HS1_CMD = 11;
static const uint8_t HS1_CLK = 6;
//SD
static const uint8_t PIN_SD_DATA3 = 13;
static const uint8_t PIN_SD_DATA2 = 12;
static const uint8_t PIN_SD_DATA1 = 4;
static const uint8_t PIN_SD_DATA0 = 2;
static const uint8_t PIN_SD_CMD = 15;
static const uint8_t PIN_SD_CLK = 14;
//SD
static const uint8_t SD_DATA3_ = 10;
static const uint8_t SD_DATA2_ = 9;
static const uint8_t SD_DATA1_ = 8;
static const uint8_t SD_DATA0_ = 7;
static const uint8_t SD_CMD_ = 11;
static const uint8_t SD_CLK_ = 6;
//FLASH
static const uint8_t FLASH_WP_N = 10;
static const uint8_t FLASH_HOLD_N = 9;
static const uint8_t FLASH_DO = 7;
static const uint8_t FLASH_DI = 8;
static const uint8_t FLASH_CS_N = 11;
static const uint8_t FLASH_CLK = 6;
//
static const uint8_t XTAL_32K_P = 32;
static const uint8_t XTAL_32K_N = 33;
//
static const uint8_t PIN_SENSOR_VP = 36;
static const uint8_t PIN_SENSOR_VN = 39;
//
static const uint8_t MTMS = 14;
static const uint8_t MTDO = 15;
static const uint8_t MTDI = 12;
static const uint8_t MTCK = 13;
//DAC
static const uint8_t DAC_2 = 26;
static const uint8_t DAC_1 = 25;
//
static const uint8_t PIN_CLK_OUT3 = 1;
static const uint8_t PIN_CLK_OUT2 = 3;
static const uint8_t PIN_CLK_OUT1 = 0;
//PSRAM
static const uint8_t SO_SIO1 = 7;
static const uint8_t SIO3 = 9;
static const uint8_t SIO2 = 10;
static const uint8_t SI_SIO0 = 8;
static const uint8_t SCLK = 17;
static const uint8_t CS_n = 16;
#endif /* Pins_Arduino_h */