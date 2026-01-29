#include "MFPT9Keyboard.h"
#include "graphics/Screen.h"

#define _TCA8418_COLS 4
#define _TCA8418_ROWS 3
#define _TCA8418_NUM_KEYS 12

#define _TCA8418_LONG_PRESS_THRESHOLD 1000
#define _TCA8418_MULTI_TAP_THRESHOLD 500

using Key = TCA8418KeyboardBase::TCA8418Key;

// External screen object
extern graphics::Screen *screen;

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
      should_backspace(false), inputMode(MULTITAP), t9_word_pending(false), t9_word_committed(false)
{
}

void MFPT9Keyboard::reset()
{
    TCA8418KeyboardBase::reset();
    t9Engine.reset();
    t9_word_pending = false;
    t9_word_committed = false;
}

void MFPT9Keyboard::toggleInputMode()
{
    if (inputMode == MULTITAP) {
        inputMode = T9_PREDICT;
        t9Engine.reset();
        t9_word_pending = false;
        t9_word_committed = false;
        LOG_INFO("T9: Switched to predictive mode");

// Show banner and set indicator
#ifdef HAS_SCREEN
        if (screen) {
            screen->showSimpleBanner("T9 Predictive", 1500);
        }
#endif
    } else {
        inputMode = MULTITAP;
        if (t9_word_pending) {
            commitT9Word();
        }
        t9_word_pending = false;
        t9_word_committed = false;
        LOG_INFO("T9: Switched to multi-tap mode");

// Show banner and remove indicator
#ifdef HAS_SCREEN
        if (screen) {
            screen->showSimpleBanner("Multi-tap", 1500);
        }
#endif
    }
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

    // Route to appropriate handler based on input mode
    if (inputMode == T9_PREDICT) {
        handleT9Input(next_key);
    } else {
        handleMultiTapInput(next_key);
    }

    // Store the current key as the last key
    last_key = next_key;
    last_tap = now;
}

void MFPT9Keyboard::handleMultiTapInput(uint8_t keyIndex)
{
    // Check if the key is the same as the last one or if the time interval has passed
    if (keyIndex != last_key || tap_interval > _TCA8418_MULTI_TAP_THRESHOLD) {
        char_idx = 0;             // Reset char index if new key or long press
        should_backspace = false; // dont backspace on new key
    } else {
        char_idx += 1;           // Cycle through characters if same key pressed
        should_backspace = true; // allow backspace on same key
    }
}

void MFPT9Keyboard::handleT9Input(uint8_t keyIndex)
{
    // In T9 mode, keys 2-9 add to the sequence
    // Key 1 is multi-tap punctuation
    // Key 0 cycles through candidates
    // Key * is backspace
    // Key # toggles mode (handled in released)

    // Handle key 1 (punctuation) multi-tap
    if (keyIndex == 0) {
        // Check if the key is the same as the last one or if the time interval has passed
        if (keyIndex != last_key || tap_interval > _TCA8418_MULTI_TAP_THRESHOLD) {
            char_idx = 0;             // Reset char index if new key or long press
            should_backspace = false; // don't backspace on new key
        } else {
            char_idx += 1;           // Cycle through characters if same key pressed
            should_backspace = true; // allow backspace on same key
        }
        return;
    }

    if (keyIndex >= 1 && keyIndex <= 10) { // Keys 2-9 (indices 1-10 in our layout)
        // Map key index to phone key number (2-9)
        // Layout: 1 2 3 # / 4 5 6 0 / 7 8 9 *
        uint8_t phoneKey = '0';
        switch (keyIndex) {
        case 1:
            phoneKey = '2';
            break;
        case 2:
            phoneKey = '3';
            break;
        case 4:
            phoneKey = '4';
            break;
        case 5:
            phoneKey = '5';
            break;
        case 6:
            phoneKey = '6';
            break;
        case 8:
            phoneKey = '7';
            break;
        case 9:
            phoneKey = '8';
            break;
        case 10:
            phoneKey = '9';
            break;
        }

        if (phoneKey != '0') {
            // If we had a committed word, this starts a new sequence
            if (t9_word_committed) {
                t9Engine.reset();
                t9_word_committed = false;
                queueEvent(' '); // Add space after previous word
            }

            // Get the previous state before adding new key
            const char *prevWord = nullptr;
            uint8_t prevLen = 0;
            bool hadPrevMatch = false;

            if (t9_word_pending) {
                if (t9Engine.hasMatches()) {
                    prevWord = t9Engine.getCurrentWord();
                    if (prevWord) {
                        prevLen = strlen(prevWord);
                        hadPrevMatch = true;
                    }
                } else {
                    // Previous was showing number sequence
                    const char *seq = t9Engine.getKeySequence();
                    if (seq) {
                        prevLen = strlen(seq);
                    }
                }
            }

            // Add key to T9 sequence
            t9Engine.addKey(phoneKey);
            t9_word_pending = true;

            // Show the predicted word on screen
            if (t9Engine.hasMatches()) {
                const char *newWord = t9Engine.getCurrentWord();
                if (newWord) {
                    // Clear previous display (word or number sequence)
                    for (uint8_t i = 0; i < prevLen; i++) {
                        queueEvent(Key::BSP);
                    }

                    // Display new prediction
                    for (uint8_t i = 0; newWord[i] != '\0'; i++) {
                        queueEvent(newWord[i]);
                    }

                    LOG_DEBUG("T9: Key %c, sequence: %s, showing: %s (%d/%d)", phoneKey, t9Engine.getKeySequence(), newWord,
                              t9Engine.getCurrentIndex() + 1, t9Engine.getCandidateCount());
                }
            } else {
                // No matches - need to show number
                if (hadPrevMatch) {
                    // Had a word match before, now no match - show entire sequence
                    for (uint8_t i = 0; i < prevLen; i++) {
                        queueEvent(Key::BSP);
                    }
                    const char *seq = t9Engine.getKeySequence();
                    if (seq) {
                        for (uint8_t i = 0; seq[i] != '\0'; i++) {
                            queueEvent(seq[i]);
                        }
                    }
                } else {
                    // Was already showing numbers, just add the new digit
                    queueEvent(phoneKey);
                }
                LOG_DEBUG("T9: Key %c, sequence: %s, no matches", phoneKey, t9Engine.getKeySequence());
            }
        }
    }
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

    // Handle mode-specific key releases
    if (inputMode == T9_PREDICT) {
        // T9 Mode handling
        if (last_key == 3) { // '#' key - short press cycles candidates, long press toggles mode
            if (held_interval > _TCA8418_LONG_PRESS_THRESHOLD) {
                toggleInputMode();
                state = Idle;
                return;
            } else {
                // Short press: Cycle through T9 candidates
                if (t9_word_pending && t9Engine.hasMatches()) {
                    cycleT9Candidate();
                    LOG_DEBUG("T9: Cycled to candidate by # key");
                }
                // If no pending word or no matches, # does nothing on short press
            }
        } else if (last_key == 7) { // '0' key - commit word and add space
            if (t9_word_pending && t9Engine.hasMatches()) {
                // Word is already on screen, just mark as committed and add space
                t9_word_committed = true;
                t9_word_pending = false;
                queueEvent(' '); // Add space after the word
                LOG_DEBUG("T9: Committed word: %s", t9Engine.getCurrentWord());
            } else if (t9_word_pending && !t9Engine.hasMatches()) {
                // No matches for the sequence - numbers are already on screen
                LOG_DEBUG("T9: No matches for sequence: %s, accepting as numbers", t9Engine.getKeySequence());
                t9Engine.reset();
                t9_word_pending = false;
                queueEvent(' '); // Add space after the numbers
            } else {
                queueEvent(' '); // Space if no T9 word pending
            }
        } else if (last_key == 11) { // '*' key - short press backspace, long press exit
            if (held_interval > _TCA8418_LONG_PRESS_THRESHOLD) {
                // Long press: exit input interface
                queueEvent(Key::ESC);
                LOG_DEBUG("T9: Long press * - exit input");
            } else {
                // Short press: backspace
                if (t9_word_committed) {
                    // Backspace on a committed word - delete character by character
                    queueEvent(Key::BSP);
                    t9_word_committed = false;
                    t9_word_pending = false;
                    t9Engine.reset();
                } else if (t9_word_pending) {
                    // Need to go back to previous prediction state
                    const char *currentWord = t9Engine.getCurrentWord();
                    uint8_t currentLen = 0;
                    if (currentWord) {
                        currentLen = strlen(currentWord);
                    } else {
                        // No matches, showing number sequence
                        const char *seq = t9Engine.getKeySequence();
                        if (seq) {
                            currentLen = strlen(seq);
                        }
                    }

                    // Remove current display
                    for (uint8_t i = 0; i < currentLen; i++) {
                        queueEvent(Key::BSP);
                    }

                    // Remove last key from sequence
                    t9Engine.backspace();

                    // Check if sequence is empty
                    const char *seq = t9Engine.getKeySequence();
                    if (!seq || seq[0] == '\0') {
                        t9_word_pending = false;
                    } else {
                        // Show new prediction or number sequence
                        if (t9Engine.hasMatches()) {
                            const char *newWord = t9Engine.getCurrentWord();
                            if (newWord) {
                                for (uint8_t i = 0; newWord[i] != '\0'; i++) {
                                    queueEvent(newWord[i]);
                                }
                            }
                        } else {
                            // Show number sequence
                            if (seq) {
                                for (uint8_t i = 0; seq[i] != '\0'; i++) {
                                    queueEvent(seq[i]);
                                }
                            }
                        }
                    }
                } else {
                    // No T9 word active - allow normal backspace for previously typed content
                    queueEvent(Key::BSP);
                    LOG_DEBUG("T9: Backspace on non-T9 content");
                }
            }
        } else if (last_key >= 1 && last_key <= 10) {
            // Keys 2-9: already handled in pressed(), prediction is displayed
            // No additional action needed on release
        } else if (last_key == 0) {
            // Key 1 - punctuation in T9 mode with multi-tap support
            // If there's a pending word, add space then punctuation
            if (t9_word_pending) {
                t9_word_committed = true;
                t9_word_pending = false;
                queueEvent(' '); // Add space after word
            }

            // Handle multi-tap for key 1 in T9 mode
            // Check if same key pressed within threshold
            if (tap_interval < _TCA8418_MULTI_TAP_THRESHOLD && should_backspace) {
                queueEvent(Key::BSP);
            }

            // Cycle through punctuation characters
            queueEvent(TCA8418TapMap[last_key][(char_idx % TCA8418TapMod[last_key])]);
            LOG_DEBUG("T9: Key 1 punctuation: %c (index %d)", TCA8418TapMap[last_key][(char_idx % TCA8418TapMod[last_key])],
                      char_idx);
        }

        state = Idle;
        return;
    }

    // Multi-tap mode handling (original behavior)
    if (tap_interval < _TCA8418_MULTI_TAP_THRESHOLD && should_backspace) {
        queueEvent(BSP);
    }
    if (held_interval > _TCA8418_LONG_PRESS_THRESHOLD) {
        // Check for mode toggle on '#' key
        if (last_key == 3 && TCA8418LongPressMap[last_key] == Key::SELECT) {
            toggleInputMode();
        } else if (last_key == 11) {
            // Long press '*' key - exit input interface (consistent with T9 mode)
            queueEvent(Key::ESC);
            LOG_DEBUG("Multi-tap: Long press * - exit input");
        } else {
            queueEvent(TCA8418LongPressMap[last_key]);
            LOG_DEBUG("Long Press Key: %i Map: %i", last_key, TCA8418LongPressMap[last_key]);
        }
    } else {
        queueEvent(TCA8418TapMap[last_key][(char_idx % TCA8418TapMod[last_key])]);
        LOG_DEBUG("Key Press: %i Index:%i if %i Map: %c", last_key, char_idx, TCA8418TapMod[last_key],
                  TCA8418TapMap[last_key][(char_idx % TCA8418TapMod[last_key])]);
    }

    state = Idle;
}

void MFPT9Keyboard::commitT9Word()
{
    // Legacy method - no longer used in real-time prediction mode
    // Kept for API compatibility
    t9_word_pending = false;
}

void MFPT9Keyboard::cycleT9Candidate()
{
    // Can cycle candidates when word is pending or committed, as long as there are matches
    if ((!t9_word_pending && !t9_word_committed) || !t9Engine.hasMatches()) {
        return;
    }

    // Get current word length to know how many backspaces to send
    const char *currentWord = t9Engine.getCurrentWord();
    if (!currentWord) {
        return;
    }
    uint8_t currentLen = strlen(currentWord);

    // Move to next candidate
    t9Engine.nextCandidate();
    const char *nextWord = t9Engine.getCurrentWord();

    if (nextWord) {
        // Clear current word
        for (uint8_t i = 0; i < currentLen; i++) {
            queueEvent(Key::BSP);
        }

        // Display new word
        for (uint8_t i = 0; nextWord[i] != '\0'; i++) {
            queueEvent(nextWord[i]);
        }

        LOG_DEBUG("T9: Cycled to candidate: %s (%d/%d)", nextWord, t9Engine.getCurrentIndex() + 1, t9Engine.getCandidateCount());
    }
}
