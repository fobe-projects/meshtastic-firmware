#define BUTTON_PIN 9

#define HAS_SCREEN 0
#define HAS_GPS 0
#undef GPS_RX_PIN
#undef GPS_TX_PIN

/**
 * LED
 */
#define PIN_LED1 (21) // P1.02
#define LED_RED PIN_LED1
#define LED_BLUE PIN_LED1
#define LED_GREEN PIN_LED1
#define LED_BUILTIN PIN_LED1
#define LED_STATE_ON 0
#define ledOff(pin) pinMode(pin, INPUT)

#define USE_LLCC68
#define LORA_SCK 6
#define LORA_MISO 2
#define LORA_MOSI 3
#define LORA_CS 10
#define LORA_DIO0 RADIOLIB_NC
#define LORA_RESET 0
#define LORA_DIO1 4
#define LORA_DIO2 RADIOLIB_NC
#define LORA_BUSY 5
#define LORA_RXEN 1
#define SX126X_CS LORA_CS
#define SX126X_DIO1 LORA_DIO1
#define SX126X_BUSY LORA_BUSY
#define SX126X_RXEN LORA_RXEN
#define SX126X_RESET LORA_RESET
#define SX126X_DIO2_AS_RF_SWITCH
// #define SX126X_DIO3_TCXO_VOLTAGE 1.8

#define UART_TX 7
#define UART_RX 8

#define I2C_SDA -1
#define I2C_SCL -1