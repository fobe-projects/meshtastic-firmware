#!/usr/bin/env python3
"""
Generate T9 Dictionary from frequency-sorted word list

This script reads a word list (one word per line, sorted by frequency)
and generates a T9 dictionary header file for the Meshtastic firmware.

Usage:
    python3 gen-t9-dict.py <input_file> <max_words> > ../src/input/T9Dictionary.h

Arguments:
    input_file: Word list file (one word per line, most frequent first)
    max_words: Maximum number of words to include (e.g., 1000)

Example:
    python3 gen-t9-dict.py google-10000-english-no-swears.txt 1000 > ../src/input/T9Dictionary.h
"""

import sys
import re
from collections import defaultdict


def char_to_t9_key(c):
    """Convert a character to its T9 key (2-9)"""
    c = c.lower()
    if c in "abc":
        return "2"
    if c in "def":
        return "3"
    if c in "ghi":
        return "4"
    if c in "jkl":
        return "5"
    if c in "mno":
        return "6"
    if c in "pqrs":
        return "7"
    if c in "tuv":
        return "8"
    if c in "wxyz":
        return "9"
    return None


def word_to_t9_sequence(word):
    """Convert a word to its T9 key sequence"""
    seq = ""
    for c in word.lower():
        key = char_to_t9_key(c)
        if key is None:
            return None  # Invalid character
        seq += key
    return seq


def is_valid_word(word):
    # """Check if word is valid (alphabetic, at least 2 characters)"""
    # if not word:
    #     return False
    # if len(word) < 2:
    #     return False
    # if not word.isalpha():
    #     return False
    return True


def generate_t9_dict(input_file, max_words):
    """Generate T9 dictionary from word list (preserves frequency order)"""
    words = []
    seen_words = set()

    print(f"# Reading words from {input_file}...", file=sys.stderr)

    with open(input_file, "r", encoding="utf-8") as f:
        for line in f:
            word = line.strip().lower()

            # Skip if not valid or already seen
            if not is_valid_word(word):
                continue
            if word in seen_words:
                continue

            # Convert to T9 sequence
            t9_seq = word_to_t9_sequence(word)
            if t9_seq is None:
                continue

            words.append((t9_seq, word))
            seen_words.add(word)

            # Stop if we have enough words
            if len(words) >= max_words:
                break

    print(f"# Collected {len(words)} unique words", file=sys.stderr)

    # Sort by T9 sequence for efficient lookup
    words.sort(key=lambda x: x[0])

    print(f"# Sorted by T9 sequence", file=sys.stderr)

    # Group by word length for better organization in output
    by_length = defaultdict(list)
    for t9_seq, word in words:
        by_length[len(word)].append((t9_seq, word))

    return words, by_length


def generate_header_file(words, by_length, max_words):
    """Generate the T9Dictionary.h header file"""

    print("#pragma once")
    print("#ifndef T9DICTIONARY_H")
    print("#define T9DICTIONARY_H")
    print()
    print("#include <Arduino.h>")
    print()
    print("/**")
    print(" * @brief Compact T9 dictionary for predictive text input")
    print(" *")
    print(
        f" * This dictionary contains {len(words)} most common English words, stored in flash memory"
    )
    print(
        " * to minimize RAM usage. Words are organized by their T9 key sequence for fast lookup."
    )
    print(" *")
    print(" * T9 Key Mapping:")
    print(" * 2: abc, 3: def, 4: ghi, 5: jkl, 6: mno, 7: pqrs, 8: tuv, 9: wxyz")
    print(" *")
    print(f" * Generated from frequency-sorted word list (top {max_words} words)")
    print(" */")
    print()
    print("// Dictionary entry: key sequence (max 8 digits) + word")
    print("struct T9Entry {")
    print('    const char *keySeq; // T9 key sequence (e.g., "4663" for "good")')
    print("    const char *word;   // The actual word")
    print("};")
    print()
    print(f"// Top {len(words)} most common English words organized by T9 key sequence")
    print("// Stored in PROGMEM (flash) to save RAM")
    print("const T9Entry T9_DICTIONARY[] PROGMEM = {")

    # Generate entries grouped by word length
    for length in sorted(by_length.keys()):
        entries = by_length[length]
        print(f"    // {length}-letter words")

        for t9_seq, word in entries:
            print(f'    {{"{t9_seq}", "{word}"}},')
        print()

    # Remove the last comma and newline
    print("};")
    print()
    print(
        "const uint16_t T9_DICTIONARY_SIZE = sizeof(T9_DICTIONARY) / sizeof(T9Entry);"
    )
    print()
    print("#endif // T9DICTIONARY_H")


def main():
    if len(sys.argv) != 3:
        print(__doc__, file=sys.stderr)
        sys.exit(1)

    input_file = sys.argv[1]
    max_words = int(sys.argv[2])

    print(f"# Generating T9 dictionary:", file=sys.stderr)
    print(f"#   Input: {input_file}", file=sys.stderr)
    print(f"#   Max words: {max_words}", file=sys.stderr)
    print(file=sys.stderr)

    # Generate dictionary
    words, by_length = generate_t9_dict(input_file, max_words)

    # Generate header file
    generate_header_file(words, by_length, max_words)

    # Print statistics
    print(f"# Generated {len(words)} entries", file=sys.stderr)
    print(f"# Word length distribution:", file=sys.stderr)
    for length in sorted(by_length.keys()):
        count = len(by_length[length])
        print(f"#   {length} letters: {count} words", file=sys.stderr)


if __name__ == "__main__":
    main()
