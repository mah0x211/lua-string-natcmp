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
#include "natcmp.h"
#include "utf8clen.h"
// lua
#include <lauxlib.h>
#include <lua.h>

static inline uint32_t utf8decode(const unsigned char *s, size_t *len)
{
    size_t illen = 0;

    switch (utf8clen(s, &illen)) {
    case 1:
        *len = 1;
        // ASCII character is case-insensitive
        return tolower(*s);
    case 2:
        *len = 2;
        return ((s[0] & 0x1F) << 6) | (s[1] & 0x3F);
    case 3:
        *len = 3;
        return ((s[0] & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F);
    case 4:
        *len = 4;
        return ((s[0] & 0x07) << 18) | ((s[1] & 0x3F) << 12) |
               ((s[2] & 0x3F) << 6) | (s[3] & 0x3F);
    default:
        // illegal byte sequence will be replaced with U+FFFD
        *len = illen;
        return 0xFFFD;
    }
}

static int nondigit_cmp_utf8(const unsigned char *a, const unsigned char *b,
                             unsigned char **end_a, unsigned char **end_b)
{
    int isdigit_a = isdigit(*a);
    int isdigit_b = isdigit(*b);

    // compare non-digit part case-insensitively
    while (!isdigit_a && !isdigit_b && *a && *b) {
        size_t alen = 0;
        size_t blen = 0;
        uint32_t ca = utf8decode(a, &alen);
        uint32_t cb = utf8decode(b, &blen);
        // compare codepoint if different
        if (ca != cb) {
            return (ca < cb) ? -1 : 1;
        }

        // skip character
        a += alen;
        b += blen;
        isdigit_a = isdigit(*a);
        isdigit_b = isdigit(*b);
    }

    // skip non-digit part
    *end_a = (unsigned char *)a;
    *end_b = (unsigned char *)b;

    // check next character
    return 0;
}

static int natcmp_lua(lua_State *L)
{
    const char *s1 = luaL_checkstring(L, 2);
    const char *s2 = luaL_checkstring(L, 3);
    lua_pushinteger(
        L, natcmp((const unsigned char *)s1, (const unsigned char *)s2, NULL));
    return 1;
}

static int natcmp_lt_lua(lua_State *L)
{
    const char *s1 = luaL_checkstring(L, 1);
    const char *s2 = luaL_checkstring(L, 2);
    lua_pushboolean(L, natcmp((const unsigned char *)s1,
                              (const unsigned char *)s2, NULL) < 0);
    return 1;
}

static int natcmp_utf8_lua(lua_State *L)
{
    const char *s1 = luaL_checkstring(L, 2);
    const char *s2 = luaL_checkstring(L, 3);
    lua_pushinteger(L, natcmp((const unsigned char *)s1,
                              (const unsigned char *)s2, nondigit_cmp_utf8));
    return 1;
}

static int natcmp_utf8_lt_lua(lua_State *L)
{
    const char *s1 = luaL_checkstring(L, 1);
    const char *s2 = luaL_checkstring(L, 2);
    lua_pushboolean(L,
                    natcmp((const unsigned char *)s1, (const unsigned char *)s2,
                           nondigit_cmp_utf8) < 0);
    return 1;
}

LUALIB_API int luaopen_string_natcmp(lua_State *L)
{
    lua_newtable(L);

    // utf8 support functions
    lua_newtable(L);
    lua_pushcfunction(L, natcmp_utf8_lt_lua);
    lua_setfield(L, -2, "lt");
    lua_newtable(L);
    lua_pushcfunction(L, natcmp_utf8_lua);
    lua_setfield(L, -2, "__call");
    lua_setmetatable(L, -2);
    lua_setfield(L, -2, "utf8");

    // ascii support functions (default)
    lua_pushcfunction(L, natcmp_lt_lua);
    lua_setfield(L, -2, "lt");
    lua_newtable(L);
    lua_pushcfunction(L, natcmp_lua);
    lua_setfield(L, -2, "__call");
    lua_setmetatable(L, -2);

    return 1;
}
