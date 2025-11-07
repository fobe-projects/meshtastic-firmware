#include "variant.h"
#include "concurrency/OSThread.h"
#include "configuration.h"
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

    // Power Enable for peripherals
    pinMode(SOLAR_POWER_PERI_EN, OUTPUT);
    digitalWrite(SOLAR_POWER_PERI_EN, HIGH);
}

// NCP15XH103F03RC thermistor parameters
#define NTC_SENSE_SAMPLES 15
#define NTC_SENSE_RESISTANCE 10000
#ifndef SOLAR_POWER_NTC_SENSE_RESOLUTION_BITS
#define SOLAR_POWER_NTC_SENSE_RESOLUTION_BITS 12
#endif
#ifndef SOLAR_POWER_NTC_SENSE_RESOLUTION
#define SOLAR_POWER_NTC_SENSE_RESOLUTION 4096.0
#endif
#ifndef SOLAR_POWER_NTC_SENSE_PULL_VOLTAGE
#define SOLAR_POWER_NTC_SENSE_PULL_VOLTAGE 3.3
#endif
#ifndef SOLAR_POWER_MAX_CHARGE_BOARD_TEMP
#define SOLAR_POWER_MAX_CHARGE_BOARD_TEMP 45.0
#endif
#ifndef SOLAR_POWER_MAX_BOOSTER_BOARD_TEMP
#define SOLAR_POWER_MAX_BOOSTER_BOARD_TEMP 50.0
#endif

const int NTC_CENTER_VALUES_SIZE = 34;
const float NTC_CENTER_TEMP[NTC_CENTER_VALUES_SIZE] = {-40, -35, -30, -25, -20, -15, -10, -5,  0,   5,  10, 15,
                                                       20,  25,  30,  35,  40,  45,  50,  55,  60,  65, 70, 75,
                                                       80,  85,  90,  95,  100, 105, 110, 115, 120, 125};
const float NTC_CENTER_R[NTC_CENTER_VALUES_SIZE] = {195.652, 148.171, 113.347, 87.559, 68.237, 53.650, 42.506, 33.892, 27.219,
                                                    22.021,  17.926,  14.674,  12.081, 10.000, 8.315,  6.948,  5.834,  4.917,
                                                    4.161,   3.535,   3.014,   2.586,  2.228,  1.925,  1.669,  1.452,  1.268,
                                                    1.110,   0.974,   0.858,   0.758,  0.672,  0.596,  0.531};

class SolarPowerThread : public concurrency::OSThread
{
  private:
    bool firstRun = true;
    unsigned long lastHeartbeat = 0;

  public:
    // Constructor - sets thread name and default interval (5 seconds)
    SolarPowerThread() : OSThread("SolarPower")
    {
        // Set initial interval to 1 second (1000ms)
        setInterval(1000);
    }

  protected:
    /**
     * @brief Main thread execution function
     * @return Next interval in milliseconds, or disable() to stop thread
     */
    int32_t runOnce() override
    {
        // First run initialization
        if (firstRun) {
            firstRun = false;
            // Solar Detect
            pinMode(SOLAR_POWER_DETECT, INPUT);

            // Boost Enable
            pinMode(SOLAR_POWER_BOOST_EN, OUTPUT);
            digitalWrite(SOLAR_POWER_BOOST_EN, HIGH);

            // Solar Enable
            pinMode(SOLAR_POWER_EN, OUTPUT);
            digitalWrite(SOLAR_POWER_EN, LOW);
            return 200; // Continue with 200 ms interval
        }

        float temperature = getTemp();
        bool solarDetected = isSolarDetected();
        bool solarEnabled = true;
        bool boostEnabled = true;
        solarEnabled = temperature <= SOLAR_POWER_MAX_CHARGE_BOARD_TEMP;
        boostEnabled = temperature <= SOLAR_POWER_MAX_BOOSTER_BOARD_TEMP;
        setBoostEnable(boostEnabled);
        setSolarEnable(solarEnabled);
        LOG_INFO("Board: temperature=%.2f, solarDetected=%s, solarEnabled=%s, boostEnabled=%s", temperature,
                 solarDetected ? "true" : "false", solarEnabled ? "true" : "false", boostEnabled ? "true" : "false");
        return 5000;
    }

    /**
     * @brief Read temperature from NTC thermistor using lookup table method
     * @return Temperature in Celsius
     */
    float getTemp()
    {
        uint32_t raw = 0;
        float ntcVoltage = 0, temp = 0;

        // Read NTC voltage multiple times for better accuracy
        for (uint32_t i = 0; i < NTC_SENSE_SAMPLES; i++) {
            raw += analogRead(SOLAR_POWER_NTC_PIN);
        }
        raw = raw / NTC_SENSE_SAMPLES;

        // Calculate voltage and resistance
        ntcVoltage = ((1000 * AREF_VOLTAGE) / pow(2, SOLAR_POWER_NTC_SENSE_RESOLUTION_BITS)) * raw;
        float ntcResistance =
            NTC_SENSE_RESISTANCE * (ntcVoltage / ((SOLAR_POWER_NTC_SENSE_PULL_VOLTAGE * 1000) - ntcVoltage)) / 1000.0; // in kOhms

        // Use lookup table method for temperature calculation
        if (ntcResistance >= NTC_CENTER_R[0]) {
            return NTC_CENTER_TEMP[0]; // Below minimum temperature (-40°C)
        }
        if (ntcResistance <= NTC_CENTER_R[NTC_CENTER_VALUES_SIZE - 1]) {
            return NTC_CENTER_TEMP[NTC_CENTER_VALUES_SIZE - 1]; // Above maximum temperature (125°C)
        }

        for (int i = 0; i < NTC_CENTER_VALUES_SIZE - 1; i++) {
            if (ntcResistance <= NTC_CENTER_R[i] && ntcResistance >= NTC_CENTER_R[i + 1]) {
                // Linear interpolation between two points
                float r1 = NTC_CENTER_R[i];        // Higher resistance (lower temp)
                float r2 = NTC_CENTER_R[i + 1];    // Lower resistance (higher temp)
                float t1 = NTC_CENTER_TEMP[i];     // Lower temperature
                float t2 = NTC_CENTER_TEMP[i + 1]; // Higher temperature

                // Interpolate temperature
                float temp = t1 + (t2 - t1) * (ntcResistance - r1) / (r2 - r1);
                return temp;
            }
        }

        // Fallback (should not reach here)
        return -273.15; // Return absolute zero as error indicator
    }

    /**
     * @brief Check if solar panel is connected
     * @return true if solar panel is detected, false otherwise
     */
    bool isSolarDetected() { return (digitalRead(SOLAR_POWER_DETECT) == LOW); }

    /**
     * @brief Enable or disable solar charging
     * @param enable true to enable, false to disable
     */
    void setSolarEnable(bool enable)
    {
        digitalWrite(SOLAR_POWER_EN, enable ? LOW : HIGH);
        LOG_INFO("Solar charging %s", enable ? "enabled" : "disabled");
    }

    /**
     * @brief Enable or disable boost converter
     * @param enable true to enable, false to disable
     */
    void setBoostEnable(bool enable)
    {
        digitalWrite(SOLAR_POWER_BOOST_EN, enable ? HIGH : LOW);
        LOG_INFO("Boost converter %s", enable ? "enabled" : "disabled");
    }
};

static SolarPowerThread *solarPower = nullptr;

void lateInitVariant()
{
    // Only create if not already created and if we're properly set up
    if (solarPower == nullptr) {
        solarPower = new SolarPowerThread();
        LOG_INFO("Solar Power thread initialized");
    }
}
