/**
 * GNSS
 */
#define HAS_GPS 0
//  #define GPS_L76K
//  #ifdef GPS_L76K
//  #define GPS_RX_PIN 40
//  #define GPS_TX_PIN 41
//  #define GPS_BAUDRATE 9600
//  #define GPS_THREAD_INTERVAL 50
//  #define PIN_SERIAL1_RX PIN_GPS_TX
//  #define PIN_SERIAL1_TX PIN_GPS_RX
//  #define PIN_GPS_STANDBY (16)
//  #endif

/**
 * NeoPixel
 */
#define HAS_NEOPIXEL
#define NEOPIXEL_COUNT 1
#define NEOPIXEL_DATA 38
#define NEOPIXEL_TYPE (NEO_GRB + NEO_KHZ800)

/*
 * Buttons
 */
#define BUTTON_PIN 0
#define BUTTON_NEED_PULLUP

/**
 * Battery
 */
#define BATTERY_PIN -1
#define ADC_CHANNEL ADC1_GPIO1_CHANNEL
#define BATTERY_SENSE_RESOLUTION_BITS 12

/**
 * I2C
 */
#define I2C_SDA 47
#define I2C_SCL 48

/**
 * Raido
 */
#define USE_SX1262
#define LORA_SCK 39
#define LORA_MISO 21
#define LORA_MOSI 42
#define LORA_CS 9
#define LORA_RXEN 17
#define LORA_RESET 8
#define LORA_DIO1 18
#define LORA_DIO2 7
#ifdef USE_SX1262
#define SX126X_CS LORA_CS
#define SX126X_DIO1 LORA_DIO1
#define SX126X_BUSY LORA_DIO2
#define SX126X_RXEN LORA_RXEN
#define SX126X_RESET LORA_RESET
#define SX126X_DIO2_AS_RF_SWITCH 1
#define SX126X_DIO3_TCXO_VOLTAGE 1.8
#define SX126X_TXEN RADIOLIB_NC
#endif