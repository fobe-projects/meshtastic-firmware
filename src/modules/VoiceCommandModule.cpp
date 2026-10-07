#if MESHTASTIC_INCLUDE_VOICE_COMMAND

#include "VoiceCommandModule.h"
#include "MeshService.h"
#include "configuration.h"
#include "input/InputBroker.h"
#include <Arduino.h>

// Edge Impulse inference library - only included in .cpp
#include <voice_command.h>

#ifdef ARCH_NRF52
#include <PDM.h>
#endif

// Global instance pointers
VoiceCommandModule *voiceCommandModule = nullptr;
VoiceCommandModule *VoiceCommandModule::instance = nullptr;

VoiceCommandModule::VoiceCommandModule() : concurrency::OSThread("VoiceCommandModule")
{
    instance = this;
    print_results = -(EI_CLASSIFIER_SLICES_PER_MODEL_WINDOW);
    LOG_INFO("VoiceCommandModule initialized");
}

VoiceCommandModule::~VoiceCommandModule()
{
    if (initialized) {
        microphoneInferenceEnd();
    }
    instance = nullptr;
}

/**
 * Calculate the energy level of an audio buffer
 * @param buffer Audio sample buffer
 * @param length Number of samples
 * @return Normalized energy value (0.0 - 1.0)
 */
float VoiceCommandModule::calculateAudioEnergy(int16_t *buffer, uint32_t length)
{
    if (!buffer || length == 0) {
        return 0.0f;
    }

    float sum = 0.0f;
    for (uint32_t i = 0; i < length; i++) {
        // Normalize to [-1.0, 1.0] range
        float normalized = buffer[i] / 32768.0f;
        // Accumulate squared values
        sum += normalized * normalized;
    }

    // Return average energy
    return sum / length;
}

int32_t VoiceCommandModule::runOnce()
{
    if (firstTime) {
        firstTime = false;

        // Display inference settings
        LOG_INFO("Voice Command Inferencing settings:");
        LOG_INFO("  Interval: %.2f ms", (float)EI_CLASSIFIER_INTERVAL_MS);
        LOG_INFO("  Frame size: %d", EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE);
        LOG_INFO("  Sample length: %d ms", EI_CLASSIFIER_RAW_SAMPLE_COUNT / 16);
        LOG_INFO("  No. of classes: %d",
                 sizeof(ei_classifier_inferencing_categories) / sizeof(ei_classifier_inferencing_categories[0]));

        // Initialize classifier
        run_classifier_init();

        // Start microphone inference
        if (microphoneInferenceStart(EI_CLASSIFIER_SLICE_SIZE)) {
            initialized = true;
            LOG_INFO("Microphone inference started successfully");
        } else {
            LOG_ERROR("Failed to allocate audio buffer (size %d)", EI_CLASSIFIER_RAW_SAMPLE_COUNT);
            return disable();
        }

        // First return, wait for data collection
        return 100; // Start processing after 100ms
    }

    if (!initialized) {
        return disable();
    }

    // Check if new data is ready (non-blocking)
    if (inference.buf_ready == 0) {
        // Data not ready yet, keep waiting
        return 10; // Check again after 10ms
    }

    // Data ready, calculate audio energy for logging
    int16_t *currentBuffer = inference.buffers[inference.buf_select ^ 1];
    float audioEnergy = calculateAudioEnergy(currentBuffer, inference.n_samples);

    // Prepare signal data
    signal_t signal;
    signal.total_length = EI_CLASSIFIER_SLICE_SIZE;
    signal.get_data = &microphoneAudioSignalGetData;
    ei_impulse_result_t result = {0};

    // Run continuous classifier - always execute, don't skip
    EI_IMPULSE_ERROR r = run_classifier_continuous(&signal, &result, debug_nn);

    // Mark buffer as processed
    inference.buf_ready = 0;

    if (r != EI_IMPULSE_OK) {
        LOG_ERROR("Failed to run classifier (%d)", r);
        return 10; // Retry after 10ms
    }

    // Accumulate inference results - use configurable window size
    if (++print_results >= DETECTION_WINDOW_SLICES) {
        // Energy check: is there sufficient energy during the entire window period
        // This allows detection of any part of a command, not just a single slice
        if (audioEnergy >= ENERGY_THRESHOLD) {
            processInferenceResult(&result);
        }
        print_results = 0;
    }

    return 50; // Check next buffer immediately
}

/**
 * PDM data ready callback
 */
void VoiceCommandModule::pdmDataReadyCallback(void)
{
#ifdef ARCH_NRF52
    if (!instance)
        return;

    int bytesAvailable = PDM.available();

    // Read to sample buffer
    int bytesRead = PDM.read((char *)&instance->sampleBuffer[0], bytesAvailable);

    if (instance->record_ready) {
        for (int i = 0; i < bytesRead >> 1; i++) {
            instance->inference.buffers[instance->inference.buf_select][instance->inference.buf_count++] =
                instance->sampleBuffer[i];

            if (instance->inference.buf_count >= instance->inference.n_samples) {
                instance->inference.buf_select ^= 1;
                instance->inference.buf_count = 0;
                instance->inference.buf_ready = 1;
            }
        }
    }
#endif
}

/**
 * Start microphone inference
 */
bool VoiceCommandModule::microphoneInferenceStart(uint32_t n_samples)
{
#ifdef ARCH_NRF52
    inference.buffers[0] = (signed short *)malloc(n_samples * sizeof(signed short));
    if (inference.buffers[0] == NULL) {
        return false;
    }

    inference.buffers[1] = (signed short *)malloc(n_samples * sizeof(signed short));
    if (inference.buffers[1] == NULL) {
        free(inference.buffers[0]);
        return false;
    }

    sampleBuffer = (signed short *)malloc((n_samples >> 1) * sizeof(signed short));
    if (sampleBuffer == NULL) {
        free(inference.buffers[0]);
        free(inference.buffers[1]);
        return false;
    }

    inference.buf_select = 0;
    inference.buf_count = 0;
    inference.n_samples = n_samples;
    inference.buf_ready = 0;

    // Configure PDM data receive callback
    PDM.onReceive(&pdmDataReadyCallback);
    PDM.setBufferSize((n_samples >> 1) * sizeof(int16_t));

    // Initialize PDM: mono, 16kHz sampling rate
    if (!PDM.begin(1, EI_CLASSIFIER_FREQUENCY)) {
        LOG_ERROR("Failed to start PDM!");
        free(inference.buffers[0]);
        free(inference.buffers[1]);
        free(sampleBuffer);
        return false;
    }

    // Set gain (0-255, default 20, max 127)
    PDM.setGain(64);

    record_ready = true;
    return true;
#else
    LOG_WARN("PDM not supported on this platform");
    return false;
#endif
}

/**
 * Get raw audio signal data
 */
int VoiceCommandModule::microphoneAudioSignalGetData(size_t offset, size_t length, float *out_ptr)
{
    if (!instance)
        return -1;

    numpy::int16_to_float(&instance->inference.buffers[instance->inference.buf_select ^ 1][offset], out_ptr, length);

    return 0;
}

/**
 * Stop PDM and free buffers
 */
void VoiceCommandModule::microphoneInferenceEnd(void)
{
#ifdef ARCH_NRF52
    PDM.end();
    if (inference.buffers[0])
        free(inference.buffers[0]);
    if (inference.buffers[1])
        free(inference.buffers[1]);
    if (sampleBuffer)
        free(sampleBuffer);

    inference.buffers[0] = nullptr;
    inference.buffers[1] = nullptr;
    sampleBuffer = nullptr;
#endif
}

/**
 * Process inference result with deduplication
 */
void VoiceCommandModule::processInferenceResult(void *result_ptr)
{
    ei_impulse_result_t &result = *(ei_impulse_result_t *)result_ptr;

    // Find classification with highest confidence
    float max_confidence = 0.0f;
    const char *detected_label = nullptr;

    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        float confidence = result.classification[ix].value;

        if (confidence > max_confidence) {
            max_confidence = confidence;
            detected_label = result.classification[ix].label;
        }
    }

    // Confidence check - higher threshold compensates for shorter window
    if (max_confidence < CONFIDENCE_THRESHOLD || !detected_label) {
        return; // Insufficient confidence, ignore
    }

    // Smart deduplication: same command only triggers once within cooldown period
    String currentCommand = String(detected_label);
    uint32_t currentTime = millis();

    if (currentCommand == lastDetectedCommand) {
        // Same command, check cooldown time
        if ((currentTime - lastDetectionTime) < COMMAND_COOLDOWN) {
            return; // Within cooldown period, ignore (sliding window overlap)
        }
    }

    // Trigger command
    LOG_INFO("*** DETECTED: %s (%.1f%%) [DSP:%dms Class:%dms] ***", detected_label, max_confidence * 100.0f, result.timing.dsp,
             result.timing.classification);

    // Update deduplication state
    lastDetectedCommand = currentCommand;
    lastDetectionTime = currentTime;

    // Process recognized commands
    if (strcmp(detected_label, "go") == 0) {
        LOG_INFO("'Go' command detected - triggering SELECT");
        InputEvent event = {.source = "VoiceCommand", .inputEvent = INPUT_BROKER_SELECT, .kbchar = 0, .touchX = 0, .touchY = 0};
        inputBroker->injectInputEvent(&event);
    } else if (strcmp(detected_label, "backward") == 0) {
        LOG_INFO("'Backward' command detected - triggering CANCEL");
        InputEvent event = {.source = "VoiceCommand", .inputEvent = INPUT_BROKER_CANCEL, .kbchar = 0, .touchX = 0, .touchY = 0};
        inputBroker->injectInputEvent(&event);
    } else if (strcmp(detected_label, "up") == 0) {
        LOG_INFO("'Up' command detected - triggering UP");
        InputEvent event = {.source = "VoiceCommand", .inputEvent = INPUT_BROKER_UP, .kbchar = 0, .touchX = 0, .touchY = 0};
        inputBroker->injectInputEvent(&event);
    } else if (strcmp(detected_label, "down") == 0) {
        LOG_INFO("'Down' command detected - triggering DOWN");
        InputEvent event = {.source = "VoiceCommand", .inputEvent = INPUT_BROKER_DOWN, .kbchar = 0, .touchX = 0, .touchY = 0};
        inputBroker->injectInputEvent(&event);
    } else if (strcmp(detected_label, "left") == 0) {
        LOG_INFO("'Left' command detected - triggering LEFT");
        InputEvent event = {.source = "VoiceCommand", .inputEvent = INPUT_BROKER_LEFT, .kbchar = 0, .touchX = 0, .touchY = 0};
        inputBroker->injectInputEvent(&event);
    } else if (strcmp(detected_label, "right") == 0) {
        LOG_INFO("'Right' command detected - triggering RIGHT");
        InputEvent event = {.source = "VoiceCommand", .inputEvent = INPUT_BROKER_RIGHT, .kbchar = 0, .touchX = 0, .touchY = 0};
        inputBroker->injectInputEvent(&event);
    } else {
        LOG_INFO("VoiceCommandModule: Command '%s' not handled", detected_label);
    }
}

#endif // !MESHTASTIC_EXCLUDE_VOICECOMMAND
