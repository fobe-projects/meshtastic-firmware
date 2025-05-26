#include "variant.h"
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
    // Setup Pins for saving power and initializing peripherals

    // DAP Serial
    pinMode(PIN_DAP_TX, INPUT);
    pinMode(PIN_DAP_RX, INPUT);

    // Setup peripheral power
    pinMode(PIN_3V3_EN, OUTPUT);
    digitalWrite(PIN_3V3_EN, HIGH);

    // Setup battery read pin
    pinMode(PIN_BATTERY_READ, OUTPUT);
    digitalWrite(PIN_BATTERY_READ, LOW);

    // Setup charge detection
    pinMode(EXT_CHRG_DETECT, INPUT);
}
