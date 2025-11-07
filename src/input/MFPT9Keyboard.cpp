#include "MFPT9Keyboard.h"

#define _TCA8418_COLS 4
#define _TCA8418_ROWS 3
#define _TCA8418_NUM_KEYS 12

#define _TCA8418_LONG_PRESS_THRESHOLD 1000
#define _TCA8418_MULTI_TAP_THRESHOLD 500

using Key = TCA8418KeyboardBase::TCA8418Key;

// Num chars per key, modulus for rotating through characters (layout: 1 2 3 # / 4 5 6 0 / 7 8 9 *)
static uint8_t TCA8418TapMod[_TCA8418_NUM_KEYS] = {13, 7, 7, 2, 7, 7, 7, 2, 9, 7, 9, 2};

static unsigned char TCA8418TapMap[_TCA8418_NUM_KEYS][13] = {
    {'.', ',', '?', '!', ':', ';', '-', '_', '\\', '/', '(', ')'}, // 1
    {'a', 'b', 'c', 'A', 'B', 'C'},                                // 2
    {'d', 'e', 'f', 'D', 'E', 'F'},                                // 3
    {'#', '@'},                                                    // #
    {'g', 'h', 'i', 'G', 'H', 'I'},                                // 4
    {'j', 'k', 'l', 'J', 'K', 'L'},                                // 5
    {'m', 'n', 'o', 'M', 'N', 'O'},                                // 6
    {' '},                                                         // 0
    {'p', 'q', 'r', 's', 'P', 'Q', 'R', 'S'},                      // 7
    {'t', 'u', 'v', 'T', 'U', 'V'},                                // 8
    {'w', 'x', 'y', 'z', 'W', 'X', 'Y', 'Z'},                      // 9
    {'*', '+'},                                                    // *
};

static unsigned char TCA8418LongPressMap[_TCA8418_NUM_KEYS] = {
    '1',         // 1
    '2',         // 2
    '3',         // 3
    Key::SELECT, // #
    '4',         // 4
    '5',         // 5
    '6',         // 6
    '0',         // 0
    '7',         // 7
    '8',         // 8
    '9',         // 9
    Key::BSP,    // *
};

MFPT9Keyboard::MFPT9Keyboard()
    : TCA8418KeyboardBase(_TCA8418_ROWS, _TCA8418_COLS), last_key(-1), next_key(-1), last_tap(0L), char_idx(0), tap_interval(0),
      should_backspace(false)
{
}

void MFPT9Keyboard::reset()
{
    TCA8418KeyboardBase::reset();
}

void MFPT9Keyboard::pressed(uint8_t key)
{
    if (state == Init || state == Busy) {
        return;
    }
    uint8_t next_key = 0;
    const int keyValue = static_cast<int>(key);

    // Decode assuming decimal row/column format (legacy behaviour).
    int row = (keyValue - 1) / 10;
    int col = (keyValue - 1) % 10;
    if (col < 0) {
        col += 10; // normalise negative remainder
    }

    bool decoded = true;

    if (row < 0 || col < 0 || row >= _TCA8418_ROWS || col >= _TCA8418_COLS) {
        // Fallback: decode using bit fields as defined in the datasheet.
        const int altRow = (keyValue >> 4) & 0x07;
        const int altCol = keyValue & 0x0F;
        row = altRow;
        col = altCol;
        decoded = false;
    }

    if (row < 0 || col < 0 || row >= _TCA8418_ROWS || col >= _TCA8418_COLS) {
        LOG_WARN("TCA8418: Ignoring raw key 0x%02X (row=%d col=%d)", key, row, col);
        state = Idle;
        return;
    }

    // Compute key index based on resolved row/column mapping
    next_key = static_cast<uint8_t>(row * _TCA8418_COLS + col);

    LOG_DEBUG("TCA8418: Key 0x%02X (%s decode) -> index %u", key, decoded ? "legacy" : "bitfield", next_key);

    state = Held;
    uint32_t now = millis();
    tap_interval = now - last_tap;
    if (tap_interval < 0) {
        // Long running, millis has overflowed.
        last_tap = 0;
        state = Busy;
        return;
    }

    // Check if the key is the same as the last one or if the time interval has passed
    if (next_key != last_key || tap_interval > _TCA8418_MULTI_TAP_THRESHOLD) {
        char_idx = 0;             // Reset char index if new key or long press
        should_backspace = false; // dont backspace on new key
    } else {
        char_idx += 1;           // Cycle through characters if same key pressed
        should_backspace = true; // allow backspace on same key
    }

    // Store the current key as the last key
    last_key = next_key;
    last_tap = now;
}

void MFPT9Keyboard::released()
{
    if (state != Held) {
        return;
    }

    if (last_key < 0 || last_key >= _TCA8418_NUM_KEYS) { // reset to idle if last_key out of bounds
        last_key = -1;
        state = Idle;
        return;
    }
    uint32_t now = millis();
    int32_t held_interval = now - last_tap;
    last_tap = now;
    if (tap_interval < _TCA8418_MULTI_TAP_THRESHOLD && should_backspace) {
        queueEvent(BSP);
    }
    if (held_interval > _TCA8418_LONG_PRESS_THRESHOLD) {
        queueEvent(TCA8418LongPressMap[last_key]);
        LOG_DEBUG("Long Press Key: %i Map: %i", last_key, TCA8418LongPressMap[last_key]);
    } else {
        queueEvent(TCA8418TapMap[last_key][(char_idx % TCA8418TapMod[last_key])]);
        LOG_DEBUG("Key Press: %i Index:%i if %i Map: %c", last_key, char_idx, TCA8418TapMod[last_key],
                  TCA8418TapMap[last_key][(char_idx % TCA8418TapMod[last_key])]);
    }
}