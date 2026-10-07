#include "T9PredictiveEngine.h"
#include <string.h>

T9PredictiveEngine::T9PredictiveEngine() : sequenceLength(0), candidateCount(0), currentCandidateIdx(0), firstMatchIdx(0)
{
    keySequence[0] = '\0';
}

void T9PredictiveEngine::addKey(uint8_t key)
{
    // Validate key (must be 2-9 for T9)
    if (key < '2' || key > '9') {
        return;
    }

    // Check if we have space
    if (sequenceLength >= MAX_SEQUENCE_LENGTH) {
        return;
    }

    // Add key to sequence
    keySequence[sequenceLength] = key;
    sequenceLength++;
    keySequence[sequenceLength] = '\0';

    // Find matching words
    findMatches();
}

void T9PredictiveEngine::backspace()
{
    if (sequenceLength == 0) {
        return;
    }

    sequenceLength--;
    keySequence[sequenceLength] = '\0';

    // Refresh matches
    if (sequenceLength > 0) {
        findMatches();
    } else {
        candidateCount = 0;
        currentCandidateIdx = 0;
        firstMatchIdx = 0;
    }
}

void T9PredictiveEngine::reset()
{
    sequenceLength = 0;
    keySequence[0] = '\0';
    candidateCount = 0;
    currentCandidateIdx = 0;
    firstMatchIdx = 0;
}

const char *T9PredictiveEngine::getCurrentWord()
{
    if (!hasMatches()) {
        return nullptr;
    }

    static char wordBuffer[16];
    uint16_t entryIdx = firstMatchIdx + currentCandidateIdx;
    getWordFromEntry(entryIdx, wordBuffer, sizeof(wordBuffer));
    return wordBuffer;
}

bool T9PredictiveEngine::nextCandidate()
{
    if (!hasMatches()) {
        return false;
    }

    if (currentCandidateIdx < candidateCount - 1) {
        currentCandidateIdx++;
        return true;
    }

    // Wrap around to first candidate
    currentCandidateIdx = 0;
    return true;
}

bool T9PredictiveEngine::previousCandidate()
{
    if (!hasMatches()) {
        return false;
    }

    if (currentCandidateIdx > 0) {
        currentCandidateIdx--;
        return true;
    }

    // Wrap around to last candidate
    currentCandidateIdx = candidateCount - 1;
    return true;
}

void T9PredictiveEngine::findMatches()
{
    candidateCount = 0;
    currentCandidateIdx = 0;
    firstMatchIdx = 0;

    if (sequenceLength == 0) {
        return;
    }

    bool foundFirst = false;

    // Linear search through dictionary
    // Note: Dictionary MUST be sorted by key sequence for early termination to work correctly
    // This is acceptable for ~500 words; binary search could be added for larger dictionaries
    for (uint16_t i = 0; i < T9_DICTIONARY_SIZE; i++) {
        if (matchesSequence(i)) {
            if (!foundFirst) {
                firstMatchIdx = i;
                foundFirst = true;
            }
            candidateCount++;
        } else if (foundFirst) {
            // Since dictionary is sorted by key sequence, we can stop after finding a non-match
            // after we've found at least one match
            break;
        }
    }
}

bool T9PredictiveEngine::matchesSequence(uint16_t entryIdx) const
{
    if (entryIdx >= T9_DICTIONARY_SIZE) {
        return false;
    }

    // Read key sequence pointer from PROGMEM
    const char *keySeqPtr = (const char *)pgm_read_ptr(&(T9_DICTIONARY[entryIdx].keySeq));

    // Compare our sequence with the dictionary entry (from PROGMEM)
    for (uint8_t i = 0; i < sequenceLength; i++) {
        char c = pgm_read_byte(keySeqPtr + i);
        if (c == '\0' || c != keySequence[i]) {
            return false;
        }
    }

    return true;
}

void T9PredictiveEngine::getWordFromEntry(uint16_t entryIdx, char *buffer, uint8_t bufferSize) const
{
    if (entryIdx >= T9_DICTIONARY_SIZE || buffer == nullptr || bufferSize == 0) {
        if (buffer && bufferSize > 0) {
            buffer[0] = '\0';
        }
        return;
    }

    // Read word from PROGMEM
    const char *wordPtr = (const char *)pgm_read_ptr(&(T9_DICTIONARY[entryIdx].word));
    strncpy_P(buffer, wordPtr, bufferSize - 1);
    buffer[bufferSize - 1] = '\0';
}
