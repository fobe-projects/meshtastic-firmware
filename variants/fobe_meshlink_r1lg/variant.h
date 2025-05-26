#ifndef _FOBE_MESHLINK_R1LG_H_
#define _FOBE_MESHLINK_R1LG_H_
/** Master clock frequency */
#define VARIANT_MCK (64000000ul)

#define USE_LFXO // Board uses 32khz crystal for LF
// #define USE_LFRC    // Board uses RC for LF

/*----------------------------------------------------------------------------
 *        Headers
 *----------------------------------------------------------------------------*/

#include "WVariant.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// Use the native nrf52 usb power detection
#define NRF_APM

// Pin definitions
#define PINS_COUNT (48)
#define NUM_DIGITAL_PINS (48)
#define NUM_ANALOG_INPUTS (1) // A5 is used for battery
#define NUM_ANALOG_OUTPUTS (0)
#define D0 (0 + 27)                 // P0.27 (RXD2)
#define D1 (0 + 26)                 // P0.26 (TXD2)
#define D2 (0 + 7)                  // P0.07
#define D3 (0 + 6)                  // P0.06
#define D4 (0 + 8)                  // P0.08
#define D5 (32 + 9)                 // P1.09
#define D6 (0 + 12)                 // P0.12
#define D7 (0 + 23)                 // P0.23
#define D8 (32 + 1)                 // P1.01
#define D9 (32 + 0)                 // P1.00
#define D10 (0 + 17)                // P0.17 (NSS2)
#define D11 (0 + 20)                // P0.20 (MOSI2)
#define D12 (0 + 22)                // P0.22 (MISO2)
#define D13 (0 + 24)                // P0.24 (SCK2)
#define D14 (0 + 2)                 // P0.02 (A0/AIN0)
#define D15 (0 + 29)                // P0.29 (A1/AIN5)
#define D16 (0 + 31)                // P0.31 (A2/AIN7)
#define D17 (0 + 30)                // P0.30 (A3/AIN6)
#define D18 (0 + 28)                // P0.28 (A4/AIN4)
#define D19 (0 + 3)                 // P0.03 (A5/AIN1)
#define D20 (0 + 5)                 // P0.05 (A6/AIN3)
#define D21 (0 + 4)                 // P0.04 (A7/AIN2)
#define D22 (32 + 8)                // P1.08
#define D23 (0 + 11)                // P0.11
#define D24 (0 + 14)                // P0.14 (MOSI1)
#define D25 (0 + 16)                // P0.16 (MISO1)
#define D26 (0 + 19)                // P0.19 (SCK1)
#define D27 (0 + 21)                // P0.21
#define D28 (32 + 3)                // P1.03
#define D29 (32 + 5)                // P1.05
#define D30 (32 + 7)                // P1.07
#define D31 (32 + 2)                // P1.02
#define D32 (32 + 4)                // P1.04
#define D33 (32 + 6)                // P1.06
#define D34 (-1)                    // NC
#define D35 (-1)                    // NC
#define D36 (32 + 10)               // P1.10
#define D37 (32 + 11)               // P1.11
#define D38 (32 + 12)               // P1.12
#define D39 (32 + 14)               // P1.14
#define D40 (32 + 13)               // P1.13
#define D41 (32 + 15)               // P1.15
#define PIN_BUILTIN_LED (0 + 25)    // P0.25
#define PIN_BUILTIN_BUTTON (32 + 7) // P1.07
#define PIN_RESET (0 + 18)          // P0.18
#define PIN_I2C_SDA (0 + 13)        // P0.13
#define PIN_I2C_SCL (0 + 15)        // P0.15
#define PIN_NFC1 (0 + 9)            // P0.09 (NFC1)
#define PIN_NFC2 (0 + 10)           // P0.10 (NFC2)
#define PIN_DAP_TX (32 + 4)         // P1.04
#define PIN_DAP_RX (32 + 6)         // P1.06

/**
 * Power
 */
#define PIN_3V3_EN D36

/*
 * LEDs
 */
#define PIN_LED1 D4
#define LED_RED PIN_LED1
#define LED_BLUE PIN_LED1
#define LED_GREEN PIN_LED1
#define LED_BUILTIN PIN_LED1
#define LED_STATE_ON 1

/*
 * Buttons
 */
#define PIN_BUTTON1 D3
#define PIN_BUTTON2 D2
#define BUTTON_SENSE_TYPE INPUT_PULLUP_SENSE

/*
 * Battery
 */
#define BATTERY_SENSE_RESOLUTION_BITS 10
#define BATTERY_PIN D20
#define PIN_BATTERY_READ D21
#define ADC_MULTIPLIER 3
#define ADC_RESOLUTION 12
#undef AREF_VOLTAGE
#define AREF_VOLTAGE 3.0
#define VBAT_AR_INTERNAL AR_INTERNAL_3_0
#define EXT_CHRG_DETECT D31
#define EXT_CHRG_DETECT_VALUE LOW

/*
 * Wire Interfaces
 */
#define HAS_WIRE 1
#define WIRE_INTERFACES_COUNT 1 // 2
#define I2C_NO_RESCAN
// #define PIN_WIRE1_SDA PIN_I2C_SDA
// #define PIN_WIRE1_SCL PIN_I2C_SCL
#define PIN_WIRE_SDA D18
#define PIN_WIRE_SCL D17

/*
 * Serial interfaces
 */
#define PIN_SERIAL2_RX (-1)
#define PIN_SERIAL2_TX (-1)
#define PIN_SERIAL1_RX D29
#define PIN_SERIAL1_TX D28

/*
 * SPI Interfaces
 */
#define SPI_INTERFACES_COUNT 2
#define PIN_SPI_MISO D25
#define PIN_SPI_MOSI D24
#define PIN_SPI_SCK D26
#define PIN_SPI1_MISO D12
#define PIN_SPI1_MOSI D11
#define PIN_SPI1_SCK D13
static const uint8_t SS = D38;
static const uint8_t MOSI = PIN_SPI_MOSI;
static const uint8_t MISO = PIN_SPI_MISO;
static const uint8_t SCK = PIN_SPI_SCK;

/*
 * LoRa
 */
#define USE_SX1262
#define SX126X_CS D38
#define SX126X_DIO1 D37
#define SX126X_BUSY D39
#define SX126X_RESET D41
#define SX126X_TXEN RADIOLIB_NC
#define SX126X_RXEN D40
#define SX126X_DIO2_AS_RF_SWITCH
#define SX126X_DIO3_TCXO_VOLTAGE 1.8

/*
 * GNSS
 */
#define GPS_L76K
#define PIN_GPS_RX PIN_SERIAL1_TX
#define PIN_GPS_TX PIN_SERIAL1_RX
#define HAS_GPS 1
#define GPS_BAUDRATE 9600
#define GPS_THREAD_INTERVAL 50
#define PIN_GPS_RESET D22
#define GPS_RESET_MODE LOW
#define PIN_GPS_STANDBY D23
#define PIN_GPS_EN D27
#define GPS_EN_ACTIVE HIGH

/**
 * E-Ink
 */
#define PIN_EINK_CS D10
#define PIN_EINK_BUSY D7
#define PIN_EINK_DC D8
#define PIN_EINK_RES D9
#define PIN_EINK_SCLK D13
#define PIN_EINK_MOSI D11

/**
 * Buzzer
 */
#define PIN_BUZZER D6

/**
 * Sensor
 */
#define PIN_LSM6DS3TR_POWER D19
#define PIN_LSM6DS3TR_INT1 D5

#ifdef __cplusplus
}
#endif

/*----------------------------------------------------------------------------
 *        Arduino objects - C++ only
 *----------------------------------------------------------------------------*/

#endif
