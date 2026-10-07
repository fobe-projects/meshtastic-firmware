#define LED_PIN 11
#define LED_STATE_ON 0

#define BUTTON_PIN 0
#define BUTTON_NEED_PULLUP

#define BATTERY_PIN 10 // A battery voltage measurement pin, voltage divider connected here to measure battery voltage
#define ADC_CHANNEL ADC1_GPIO10_CHANNEL
#define ADC_ATTENUATION ADC_ATTEN_DB_12 // lower dB for high resistance voltage divider
#define ADC_MULTIPLIER 1.73

#define USE_SSD1306 // Builtin SSD1306 display
#define I2C_SDA 14  // I2C pins for this board
#define I2C_SCL 13
#define SCREEN_MIRROR

#define VEXT_ENABLE 12 // active high, powers the oled display
#define VEXT_ON_VALUE LOW

#define USE_SX1262

#define LORA_RESET 44
#define LORA_DIO1 42
#define LORA_BUSY 43
#define LORA_SCK 40
#define LORA_MISO 41
#define LORA_MOSI 39
#define LORA_CS 45

#ifdef USE_SX1262
#define SX126X_CS LORA_CS
#define SX126X_DIO1 LORA_DIO1
#define SX126X_BUSY LORA_BUSY
#define SX126X_RESET LORA_RESET

#define SX126X_DIO2_AS_RF_SWITCH
#define SX126X_ANT_SW 46
#define SX126X_RXEN RADIOLIB_NC
#define SX126X_TXEN RADIOLIB_NC
#define SX126X_DIO3_TCXO_VOLTAGE 1.8
#endif

/*
 * GNSS
 */
#define GPS_RX_PIN 38
#define GPS_TX_PIN 37
#define PIN_SERIAL1_RX PIN_GPS_RX
#define PIN_SERIAL1_TX PIN_GPS_TX
#define HAS_GPS 1
#define GPS_BAUDRATE 9600
#define GPS_THREAD_INTERVAL 50
#define PIN_GPS_RESET 36
#define GPS_RESET_MODE LOW
#define PIN_GPS_STANDBY 35
#define PIN_GPS_EN 1
#define GPS_EN_ACTIVE HIGH