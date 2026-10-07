#include "variant.h"
#include "Arduino.h"
#include "nrf.h"
#include "wiring_constants.h"
#include "wiring_digital.h"

const uint32_t g_ADigitalPinMap[] = {
    // P0
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,

    // P1
    32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47};

void initVariant()
{
    // LED
    pinMode(PIN_BUILTIN_LED, OUTPUT);
    digitalWrite(PIN_BUILTIN_LED, HIGH);

    // Button
    pinMode(PIN_BUTTON1, INPUT);

    // Battery Sense
    pinMode(BATTERY_PIN, INPUT);

    // Charging Detection
    pinMode(EXT_CHRG_DETECT, INPUT);

    // Peripheral Power
    pinMode(PIN_PERI_EN, OUTPUT);
    digitalWrite(PIN_PERI_EN, HIGH);
}

void variant_shutdown()
{
    nrf_gpio_cfg_default(PIN_LED1);
    nrf_gpio_cfg_default(EXT_CHRG_DETECT);
    nrf_gpio_cfg_default(PIN_WIRE_SDA);
    nrf_gpio_cfg_default(PIN_WIRE_SCL);
    nrf_gpio_cfg_default(PIN_SERIAL1_RX);
    nrf_gpio_cfg_default(PIN_SERIAL1_TX);
    nrf_gpio_cfg_default(PIN_GPS_EN);
    nrf_gpio_cfg_default(PIN_GPS_STANDBY);
    nrf_gpio_cfg_default(PIN_BUZZER);
    nrf_gpio_cfg_default(PIN_PERI_EN);
    nrf_gpio_cfg_default(PIN_ROTARY_ENCODER_A);
    nrf_gpio_cfg_default(PIN_ROTARY_ENCODER_B);
    nrf_gpio_cfg_default(PIN_ROTARY_ENCODER_S);
}
