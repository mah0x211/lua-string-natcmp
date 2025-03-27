/**
 *  Copyright (C) 2025 Masatoshi Fukunaga
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to
 *  deal in the Software without restriction, including without limitation the
 *  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 *  sell copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *  IN THE SOFTWARE.
 *
 */

#ifndef utf8clen_h
#define utf8clen_h

#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>

/**
 * @brief Determine the length of a single UTF-8 character
 *
 * This function computes the length (in bytes) of a single UTF-8 character
 * starting at the given position. It follows the guidelines from the Unicode
 * Standard Version 15.0, specifically Table 3-7 of the Core Specification.
 *
 * @param s Pointer to a NULL-terminated string containing a UTF-8 character
 * @param illlen Pointer to a size_t that will receive the number of illegal
 * bytes if an invalid UTF-8 sequence is detected
 *
 * @return The length of the UTF-8 character in bytes (1-4) if valid,
 *         0 if the UTF-8 sequence is invalid (and illlen is set to the number
 * of illegal bytes), or SIZE_MAX if parameters are invalid (and errno is set to
 * EINVAL)
 */
static inline size_t utf8clen(const unsigned char *s, size_t *illlen)
{
    if (!s || !illlen) {
        errno = EINVAL;
        return SIZE_MAX;
    }

    //
    // The Unicode Standard
    // Version 15.0 – Core Specification
    //
    // Chapter 3
    //  Conformance
    // https://www.unicode.org/versions/Unicode15.0.0/ch03.pdf
    //
    // Table 3-7. Well-Formed UTF-8 Byte Sequences
    //
    // illegal byte sequence will be replaced with U+FFFD ("\xEF\xBF\xBD")
    //

    unsigned char c = *s;
    // 1 byte: 00-7F (ASCII)
    if (c <= 0x7F) {
        return 1;
    }

#define is_utf8tail(c) (((c) & 0xC0) == 0x80)

#define is_utf8firstb(c)                                                       \
    (c <= 0x7F ||                /* 00-7F: ASCII */                            \
     (c >= 0xC2 && c <= 0xDF) || /* C2-DF: 2 byte */                           \
     (c >= 0xE0 && c <= 0xEF) || /* E0-EF: 3 byte */                           \
     (c >= 0xF0 && c <= 0xF4))   /* F0-F4: 4 byte */

#define count_illegal_sequences(max)                                           \
    do {                                                                       \
        size_t len = 1;                                                        \
        c          = s[len];                                                   \
        while (c && len < (size_t)max && !is_utf8firstb(c)) {                  \
            len++;                                                             \
            c = s[len];                                                        \
        }                                                                      \
        *illlen = len;                                                         \
    } while (0)

    // 2 byte: C2-DF 80-BF
    if (c >= 0xC2 && c <= 0xDF) {
        if (is_utf8tail(s[1])) {
            return 2;
        }
        count_illegal_sequences(2);
        return 0;
    }

    // 3 byte: E0 A0-BF 80-BF
    if (c == 0xE0) {
        if (s[1] >= 0xA0 && s[1] <= 0xBF && is_utf8tail(s[2])) {
            return 3;
        }
        count_illegal_sequences(3);
        return 0;
    }

    // 3 byte: E1-EC 2(80-BF)
    //         EE-EF 2(80-BF)
    if ((c >= 0xE1 && c <= 0xEC) || (c >= 0xEE && c <= 0xEF)) {
        if (is_utf8tail(s[1]) && is_utf8tail(s[2])) {
            return 3;
        }
        count_illegal_sequences(3);
        return 0;
    }

    // 3 byte: ED 80-9F 80-BF
    if (c == 0xED) {
        if (s[1] >= 0x80 && s[1] <= 0x9F && is_utf8tail(s[2])) {
            return 3;
        }
        count_illegal_sequences(3);
        return 0;
    }

    // 4 byte: F0 90-BF 2(80-BF)
    if (c == 0xF0) {
        if (s[1] >= 0x90 && s[1] <= 0xBF && is_utf8tail(s[2]) &&
            is_utf8tail(s[3])) {
            return 4;
        }
        count_illegal_sequences(4);
        return 0;
    }

    // 4 byte: F1-F3 3(80-BF)
    if (c >= 0xF1 && c <= 0xF3) {
        if (is_utf8tail(s[1]) && is_utf8tail(s[2]) && is_utf8tail(s[3])) {
            return 4;
        }
        count_illegal_sequences(4);
        return 0;
    }

    // 4 byte: F4 80-8F 2(80-BF)
    if (c == 0xF4) {
        if (s[1] >= 0x80 && s[1] <= 0x8F && is_utf8tail(s[2]) &&
            is_utf8tail(s[3])) {
            return 4;
        }
        count_illegal_sequences(4);
        return 0;
    }

    count_illegal_sequences(SIZE_MAX);
    return 0;

#undef is_utf8tail
#undef is_utf8firstb
#undef count_illegal_sequences
}

#endif
