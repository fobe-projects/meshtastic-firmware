#ifndef _FOBE_QUILL_NRF52840_MESH_H_
#define _FOBE_QUILL_NRF52840_MESH_H_
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
#define A0 (0 + 3)
#define A1 (0 + 28)
#define A2 (0 + 2)
#define A3 (0 + 29)
#define A4 (0 + 31)
#define A5 (0 + 30)
#define D0 (32 + 12)
#define D1 (32 + 11)
#define D2 (32 + 14)
#define D3 (32 + 0)
#define D4 (0 + 24)
#define D5 (0 + 22)
#define D6 (0 + 20)
#define D7 (0 + 17)
#define D8 (0 + 15)
#define D9 (0 + 13)
#define D10 (0 + 16)
#define D11 (0 + 14)
#define D12 (0 + 11)
#define D13 (32 + 8)
#define SPI_SCK (0 + 23)
#define SPI_MOSI (0 + 21)
#define SPI_MISO (0 + 19)
#define PIN_BUILTIN_LED (32 + 15)
#define PIN_RESET (0 + 18)
#define PIN_I2C_SDA D4
#define PIN_I2C_SCL D5

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
#define PIN_BUTTON1 (32 + 10)
#define BUTTON_SENSE_TYPE INPUT_PULLUP_SENSE

/*
 * Battery
 */
#define BATTERY_PIN (0 + 5)
#define BATTERY_SENSE_RESOLUTION_BITS 12
#define BATTERY_SENSE_RESOLUTION 4096.0
#undef AREF_VOLTAGE
#define AREF_VOLTAGE 3.0
#define VBAT_AR_INTERNAL AR_INTERNAL_3_0
#define ADC_MULTIPLIER 1.73
#define EXT_CHRG_DETECT (32 + 13)
#define EXT_CHRG_DETECT_VALUE LOW

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
#define PIN_SERIAL1_RX (32 + 5)
#define PIN_SERIAL1_TX (32 + 7)

/*
 * Screen
 */
#define USE_ST7789

#define ST7789_NSS (0 + 12)
#define ST7789_RS (0 + 6)  // DC
#define ST7789_SDA (0 + 8) // MOSI
#define ST7789_SCK (32 + 9)
#define ST7789_RESET (0 + 26)
#define ST7789_MISO -1
#define ST7789_BUSY -1
#define VTFT_CTRL (0 + 7)
#define VTFT_LEDA (0 + 27)
// #define ST7789_BL (32+6)
#define TFT_BACKLIGHT_ON LOW
#define ST7789_SPI_HOST SPI1_HOST
// #define TFT_BL (32+6)
#define SPI_FREQUENCY 40000000
#define SPI_READ_FREQUENCY 16000000
#define TFT_HEIGHT 135
#define TFT_WIDTH 240
#define TFT_OFFSET_X 0
#define TFT_OFFSET_Y 0
#define TFT_OFFSET_ROTATION 2
#define SCREEN_ROTATE
#define SCREEN_TRANSITION_FRAMERATE 5 // fps

/*
 * SPI Interfaces
 */
#define SPI_INTERFACES_COUNT 2
#define PIN_SPI_MISO SPI_MISO
#define PIN_SPI_MOSI SPI_MOSI
#define PIN_SPI_SCK SPI_SCK
#define PIN_SPI1_MISO (1)
#define PIN_SPI1_MOSI ST7789_SDA
#define PIN_SPI1_SCK ST7789_SCK
static const uint8_t SS = (32 + 1);
static const uint8_t MOSI = PIN_SPI_MOSI;
static const uint8_t MISO = PIN_SPI_MISO;
static const uint8_t SCK = PIN_SPI_SCK;

/*
 * LoRa
 */
#define USE_SX1262
#define SX126X_CS (32 + 1)
#define SX126X_DIO1 (32 + 2)
#define SX126X_BUSY (32 + 4)
#define SX126X_RESET (32 + 6)
#define SX126X_TXEN RADIOLIB_NC
#define SX126X_RXEN RADIOLIB_NC
#define SX126X_ANT_SW (0 + 25)
#define SX126X_DIO2_AS_RF_SWITCH
#define SX126X_DIO3_TCXO_VOLTAGE 1.8

/*
 * GNSS
 */
#define PIN_GPS_RX PIN_SERIAL1_TX
#define PIN_GPS_TX PIN_SERIAL1_RX
#define HAS_GPS 1
#define GPS_BAUDRATE 9600
#define GPS_THREAD_INTERVAL 50
#define PIN_GPS_RESET (0 + 9)
#define GPS_RESET_MODE LOW
#define PIN_GPS_STANDBY (0 + 10)
#define PIN_GPS_EN (32 + 3)
#define GPS_EN_ACTIVE HIGH
#define GPS_THREAD_INTERVAL 50

#ifdef __cplusplus
}
#endif

/*----------------------------------------------------------------------------
 *        Arduino objects - C++ only
 *----------------------------------------------------------------------------*/

#endif
