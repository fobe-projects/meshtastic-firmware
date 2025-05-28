#ifndef _FOBE_MESHLINK_SOLAR_POWER_H_
#define _FOBE_MESHLINK_SOLAR_POWER_H_
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
#define A0 (0 + 2)
#define A1 (0 + 31)
#define A2 (0 + 29)
#define A3 (0 + 3)
#define A4 (0 + 28)
#define A5 (0 + 30)
#define SPI_SCK (0 + 13)
#define SPI_MOSI (0 + 15)
#define SPI_MISO (0 + 17)
#define D0 (0 + 20)
#define D1 (0 + 22)
#define D2 (0 + 24)
#define D3 (0 + 6)
#define D4 (0 + 8)
#define D5 (32 + 9)
#define D6 (32 + 11)
#define D9 (32 + 10)
#define D10 (32 + 12)
#define D11 (32 + 14)
#define D12 (32 + 13)
#define D13 (32 + 15)
#define PIN_BUILTIN_LED (0 + 12)
#define PIN_BUILTIN_BUTTON (32 + 3)
#define PIN_RESET (0 + 18)
#define PIN_I2C_SDA D5
#define PIN_I2C_SCL D6
#define PIN_NFC1 (0 + 9)
#define PIN_NFC2 (0 + 10)

/**
 * Power
 */
#define PIN_3V3_EN D3
#define PIN_BOOSTER_EN D2

/*
 * LEDs
 */
#define PIN_LED1 PIN_BUILTIN_LED
#define LED_RED PIN_LED1
#define LED_BLUE PIN_LED1
#define LED_GREEN PIN_LED1
#define LED_BUILTIN PIN_LED1
#define LED_STATE_ON 0

/*
 * Buttons
 */
#define PIN_BUTTON1 PIN_BUILTIN_BUTTON
#define BUTTON_SENSE_TYPE INPUT_PULLUP_SENSE

/*
 * Battery
 */
#define BATTERY_SENSE_RESOLUTION_BITS 12
#define BATTERY_PIN (0 + 4)
#define ADC_MULTIPLIER 1.73
#define ADC_RESOLUTION 12
#undef AREF_VOLTAGE
#define AREF_VOLTAGE 3.0
#define VBAT_AR_INTERNAL AR_INTERNAL_3_0

/*
 * Wire Interfaces
 */
#define HAS_WIRE 1
#define WIRE_INTERFACES_COUNT 1
#define I2C_NO_RESCAN
#define PIN_WIRE_SDA PIN_I2C_SDA
#define PIN_WIRE_SCL PIN_I2C_SCL

/*
 * Serial interfaces
 */
#define PIN_SERIAL2_RX (-1)
#define PIN_SERIAL2_TX (-1)
#define PIN_SERIAL1_RX (-1)
#define PIN_SERIAL1_TX (-1)

/*
 * SPI Interfaces
 */
#define SPI_INTERFACES_COUNT 1
#define PIN_SPI_MISO (0 + 19)
#define PIN_SPI_MOSI (0 + 21)
#define PIN_SPI_SCK (0 + 23)
static const uint8_t SS = (32 + 1);
static const uint8_t MOSI = PIN_SPI_MOSI;
static const uint8_t MISO = PIN_SPI_MISO;
static const uint8_t SCK = PIN_SPI_SCK;

/*
 * LoRa
 */
#define USE_SX1262
#define SX126X_CS (32 + 1)
#define SX126X_DIO1 (32 + 6)
#define SX126X_BUSY (32 + 4)
#define SX126X_RESET (32 + 2)
#define SX126X_TXEN RADIOLIB_NC
#define SX126X_RXEN (0 + 25)
#define SX126X_DIO2_AS_RF_SWITCH
#define SX126X_DIO3_TCXO_VOLTAGE 1.8

#ifdef __cplusplus
}
#endif

/*----------------------------------------------------------------------------
 *        Arduino objects - C++ only
 *----------------------------------------------------------------------------*/

#endif
