#include "T9PredictiveEngine.h"
#include "TCA8418KeyboardBase.h"

/**
 * @brief 3x4 keypad with 3 columns and 4 rows
 *
 * Supports two input modes:
 * - Multi-tap mode (default): Press keys multiple times to cycle through characters
 * - T9 predictive mode: Press key sequence to get word predictions
 *
 * Mode switching: Long press '#' key
 * In T9 mode: Press '0' to cycle through word candidates
 */
class MFPT9Keyboard : public TCA8418KeyboardBase
{
  public:
    enum InputMode {
        MULTITAP,  // Traditional multi-tap input
        T9_PREDICT // T9 predictive text input
    };

    MFPT9Keyboard();
    void reset(void) override;

    /**
     * @brief Switch between multi-tap and T9 predictive modes
     */
    void toggleInputMode();

    /**
     * @brief Get current input mode
     */
    InputMode getInputMode() const { return inputMode; }

  protected:
    void pressed(uint8_t key) override;
    void released(void) override;

    // Multi-tap mode variables
    int8_t last_key;
    int8_t next_key;
    uint32_t last_tap;
    uint8_t char_idx;
    int32_t tap_interval;
    bool should_backspace;

    // T9 predictive mode
    InputMode inputMode;
    T9PredictiveEngine t9Engine;

    // T9 State machine:
    // - Both false: idle, no T9 word active
    // - pending=true, committed=false: building word, prediction shown in real-time
    // - pending=false, committed=true: word committed with space, can still cycle candidates
    // - Both true: invalid state (should not occur)
    bool t9_word_pending;   // True when actively building a T9 word (keys 2-9 pressed)
    bool t9_word_committed; // True after committing word with '0' (can still cycle with '#')

    // Helper methods
    void handleMultiTapInput(uint8_t keyIndex);
    void handleT9Input(uint8_t keyIndex);
    void commitT9Word();     // Legacy - kept for API compatibility
    void cycleT9Candidate(); // Cycle through word predictions
};
