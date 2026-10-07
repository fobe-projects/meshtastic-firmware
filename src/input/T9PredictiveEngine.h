#pragma once
#ifndef T9PREDICTIVEENGINE_H
#define T9PREDICTIVEENGINE_H

#include "T9Dictionary.h"
#include <Arduino.h>

/**
 * @brief T9 Predictive Text Engine
 *
 * Provides word prediction based on numeric key sequences.
 * Uses a compact dictionary stored in flash memory to minimize RAM usage.
 */
class T9PredictiveEngine
{
  public:
    T9PredictiveEngine();

    /**
     * @brief Add a key press to the current sequence
     * @param key The numeric key (2-9)
     */
    void addKey(uint8_t key);

    /**
     * @brief Remove the last key from the sequence
     */
    void backspace();

    /**
     * @brief Clear the current key sequence
     */
    void reset();

    /**
     * @brief Get the current word prediction
     * @return Current predicted word or nullptr if no match
     */
    const char *getCurrentWord();

    /**
     * @brief Move to the next word candidate
     * @return true if there is another candidate, false otherwise
     */
    bool nextCandidate();

    /**
     * @brief Move to the previous word candidate
     * @return true if there is a previous candidate, false otherwise
     */
    bool previousCandidate();

    /**
     * @brief Get the current key sequence
     * @return The key sequence string
     */
    const char *getKeySequence() const { return keySequence; }

    /**
     * @brief Get the number of matching words
     * @return Number of candidates
     */
    uint16_t getCandidateCount() const { return candidateCount; }

    /**
     * @brief Get the current candidate index
     * @return Current index (0-based)
     */
    uint16_t getCurrentIndex() const { return currentCandidateIdx; }

    /**
     * @brief Check if there are any matches
     * @return true if matches found
     */
    bool hasMatches() const { return candidateCount > 0; }

  private:
    static const uint8_t MAX_SEQUENCE_LENGTH = 10;

    char keySequence[MAX_SEQUENCE_LENGTH + 1]; // Current key sequence
    uint8_t sequenceLength;                    // Length of current sequence

    uint16_t candidateCount;      // Number of matching words
    uint16_t currentCandidateIdx; // Current candidate index
    uint16_t firstMatchIdx;       // Index of first match in dictionary

    /**
     * @brief Find all matching words for the current sequence
     */
    void findMatches();

    /**
     * @brief Compare key sequence with dictionary entry
     * @param entryIdx Index in the dictionary
     * @return true if matches
     */
    bool matchesSequence(uint16_t entryIdx) const;

    /**
     * @brief Get word from dictionary entry
     * @param entryIdx Index in the dictionary
     * @param buffer Buffer to store the word
     * @param bufferSize Size of the buffer
     */
    void getWordFromEntry(uint16_t entryIdx, char *buffer, uint8_t bufferSize) const;
};

#endif // T9PREDICTIVEENGINE_H
