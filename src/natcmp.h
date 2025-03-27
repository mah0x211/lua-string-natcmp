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
 */

#ifndef natcmp_h
#define natcmp_h

#include <ctype.h>
#include <stddef.h>
#include <string.h>
#include <strings.h>

/**
 * natcmp_nondigit_cmp_func_t
 *
 * Callback function type definition for comparing non-digit portions of
 * strings.
 *
 * @param a     First string to compare
 * @param b     Second string to compare
 * @param end_a Pointer to store the end position of comparison in string A
 * @param end_b Pointer to store the end position of comparison in string B
 * @return int  result (negative if a < b, positive if a > b, 0 if equal)
 */
typedef int (*natcmp_nondigit_cmp_func_t)(const unsigned char *a,
                                          const unsigned char *b,
                                          unsigned char **end_a,
                                          unsigned char **end_b);

/**
 * natcmp_nondigit_cmp_ascii
 *
 * Compares the non-digit portions of two strings using a case-insensitive ASCII
 * comparison. This function is designed to be used as a callback for the natcmp
 * function.
 *
 * Algorithm:
 * 1. Scan both strings until a digit or end of string is encountered
 * 2. Compare the non-digit portions case-insensitively
 * 3. If non-digit parts differ, return the comparison result
 * 4. If lengths differ but content is equal up to the shorter length,
 *    shorter string is considered less
 * 5. Update end_a and end_b to point to the first digit or end of each string
 *
 * @param a      First string to compare
 * @param b      Second string to compare
 * @param end_a  Output parameter to store position of first digit or end in
 * string A
 * @param end_b  Output parameter to store position of first digit or end in
 * string B
 * @return int   Comparison result (-1 if a < b, 1 if a > b, 0 if equal)
 */
static inline int natcmp_nondigit_cmp_ascii(const unsigned char *a,
                                            const unsigned char *b,
                                            unsigned char **end_a,
                                            unsigned char **end_b)
{
    // calculate length of non-digit part
    const unsigned char *pa = a;
    const unsigned char *pb = b;
    while (*pa && !isdigit(*pa)) {
        pa++;
    }
    while (*pb && !isdigit(*pb)) {
        pb++;
    }

    size_t len_a = (size_t)(pa - a);
    size_t len_b = (size_t)(pb - b);
    // compare non-digit part case-insensitively
    int cmp      = strncasecmp((const char *)a, (const char *)b,
                          (len_a < len_b) ? len_a : len_b);
    if (cmp != 0) {
        // non-digit part is different
        return (cmp < 0) ? -1 : 1;
    } else if (len_a != len_b) {
        // length of non-digit part is different
        return (len_a < len_b) ? -1 : 1;
    }

    // skip non-digit part
    *end_a = (unsigned char *)pa;
    *end_b = (unsigned char *)pb;

    // check next character
    return 0;
}

/**
 * natcmp
 *
 * Compares two strings using natural order comparison.
 * Unlike standard string comparison, this treats numeric portions as numbers.
 * Example: "file2.txt" comes before "file10.txt"
 *
 * Algorithm:
 * 1. Non-digit portions are compared using the provided callback function
 * 2. Digit portions are compared as numeric values
 * 3. If numeric values are equal, the one with fewer digits comes first
 *
 * @param a         First string to compare
 * @param b         Second string to compare
 * @param compare   Callback function for comparing non-digit portions
 * @return int      Comparison result (-1=a is less, 0=equal, 1=a is greater)
 */
static inline int natcmp(const unsigned char *a, const unsigned char *b,
                         natcmp_nondigit_cmp_func_t compare)
{
    if (!compare) {
        // default to ASCII comparison if no callback is provided
        compare = natcmp_nondigit_cmp_ascii;
    }

    while (*a && *b) {
        int isdigit_a = isdigit(*a);
        int isdigit_b = isdigit(*b);
        if (!isdigit_a && !isdigit_b) {
            // compare non-digit part
            unsigned char *end_a = NULL;
            unsigned char *end_b = NULL;
            int res              = compare(a, b, &end_a, &end_b);
            if (res != 0) {
                // non-digit part is different
                return (res < 0) ? -1 : 1;
            }
            // check next character
            a = end_a;
            b = end_b;
            if (!*a || !*b) {
                // stop if end of string is reached
                break;
            }

            isdigit_a = isdigit(*a);
            isdigit_b = isdigit(*b);
        }

        if (isdigit_a != isdigit_b) {
            // number is less than non-digit character
            // so, a is less than b
            return isdigit_a ? -1 : 1;
        }

        struct {
            const unsigned char *head; // head of number part with leading zeros
            const unsigned char
                *digits; // head of number part without leading zeros
            size_t len;  // length of number part
            const unsigned char *tail; // tail of number part
        } an, bn;
        an.head = an.tail = a;
        bn.head = bn.tail = b;

#define natcmp_skip_leading_zeros(s)                                           \
    while (*(s) == '0' && (s)[1] && isdigit((s)[1])) {                         \
        (s)++;                                                                 \
    }
        natcmp_skip_leading_zeros(an.tail);
        natcmp_skip_leading_zeros(bn.tail);
#undef natcmp_skip_leading_zeros

        an.digits = an.tail;
        bn.digits = bn.tail;

#define natcmp_skip_digits(s)                                                  \
    while (*(s) && isdigit(*(s))) {                                            \
        (s)++;                                                                 \
    }
        natcmp_skip_digits(an.tail);
        natcmp_skip_digits(bn.tail);
#undef natcmp_skip_digits

        // calculate length of number part without leading zeros
        an.len = (size_t)(an.tail - an.digits);
        bn.len = (size_t)(bn.tail - bn.digits);

        // compare number part
        if (an.len != bn.len) {
            // number part is different
            return (an.len < bn.len) ? -1 : 1;
        }

        // compare digits
        int cmp =
            strncmp((const char *)an.digits, (const char *)bn.digits, an.len);
        if (cmp != 0) {
            // number part is different
            return (cmp < 0) ? -1 : 1;
        }

        // compare length of number part with leading zeros
        an.len = (size_t)(an.tail - an.head);
        bn.len = (size_t)(bn.tail - bn.head);
        if (an.len != bn.len) {
            // longest number part is greater
            return (an.len < bn.len) ? -1 : 1;
        }

        // whole number part is same
        a = an.tail;
        b = bn.tail;

        // continue to next character
    }

    if (*b) {
        // a is shorter than b
        return -1;
    } else if (*a) {
        // a is longer than b
        return 1;
    }
    // a and b are same
    return 0;
}

#endif /* natcmp_h */
