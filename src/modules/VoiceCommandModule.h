#pragma once

#if MESHTASTIC_INCLUDE_VOICE_COMMAND

#include "concurrency/OSThread.h"
#include "configuration.h"
#include <Arduino.h>

/** Audio buffers, pointers and selectors */
typedef struct {
    signed short *buffers[2];
    unsigned char buf_select;
    unsigned char buf_ready;
    unsigned int buf_count;
    unsigned int n_samples;
} inference_t;

/**
 * VoiceCommandModule - Voice recognition module
 * Performs continuous voice recognition using Edge Impulse trained model
 * Recognition results are processed and injected as input events
 */
class VoiceCommandModule : private concurrency::OSThread
{
  private:
    // === Tunable Parameters ===
    // Detection window size: 2 slices (500ms) balances speed and accuracy
    const int DETECTION_WINDOW_SLICES = 2; // 2 slices = 500ms, 4 slices = 1 second

    // Audio energy threshold for voice activity detection (RMS normalized)
    const float ENERGY_THRESHOLD = 0.01f; // Lower threshold to avoid missing commands

    // Confidence threshold for command recognition (compensated for shorter window)
    const float CONFIDENCE_THRESHOLD = 0.88f; // 88% threshold compensates for reduced data

    // Cooldown period to prevent duplicate triggers from sliding window overlap
    const uint32_t COMMAND_COOLDOWN = 600; // 600ms (2 slices × 250ms × 2x margin)

    // === Runtime State ===
    bool firstTime = true;
    bool initialized = false;
    bool debug_nn = false; // Enable neural network debug output
    int print_results;

    // Command deduplication state
    String lastDetectedCommand = "";
    uint32_t lastDetectionTime = 0;

    // Audio inference state
    inference_t inference;
    bool record_ready = false;
    signed short *sampleBuffer = nullptr;

    // === Static Members ===
    // PDM callback function (must be static)
    static VoiceCommandModule *instance;
    static void pdmDataReadyCallback(void);

    // === Audio Inference Functions ===
    bool microphoneInferenceStart(uint32_t n_samples);
    void microphoneInferenceEnd(void);
    static int microphoneAudioSignalGetData(size_t offset, size_t length, float *out_ptr);

    // Audio energy detection
    float calculateAudioEnergy(int16_t *buffer, uint32_t length);

    // Process inference result - uses void* to avoid including Edge Impulse headers
    void processInferenceResult(void *result_ptr);

  public:
    VoiceCommandModule();
    virtual ~VoiceCommandModule();

  protected:
    virtual int32_t runOnce() override;
};

extern VoiceCommandModule *voiceCommandModule;

#endif // !USE_VOICE_COMMAND
