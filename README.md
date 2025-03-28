# lua-string-natcmp

[![test](https://github.com/mah0x211/lua-string-natcmp/actions/workflows/test.yml/badge.svg)](https://github.com/mah0x211/lua-string-natcmp/actions/workflows/test.yml)
[![codecov](https://codecov.io/gh/mah0x211/lua-string-natcmp/branch/master/graph/badge.svg)](https://codecov.io/gh/mah0x211/lua-string-natcmp)

Natural order string comparison function for Lua with full UTF-8 support.

## What is Natural Sorting?

Natural sorting is a string sorting algorithm that orders text containing numbers in a way that corresponds to human expectations. For example:

Standard lexicographic sorting: `"file1.txt", "file10.txt", "file2.txt"`

Natural order sorting: `"file1.txt", "file2.txt", "file10.txt"`

This library implements natural order comparison with full UTF-8 support and case-insensitive ASCII comparison.

## Features

- **Case-insensitive comparison**: ASCII alphabetical characters are compared case-insensitively
- **Natural numeric comparison**: Compares numeric sequences as numbers (10 > 2) without converting to actual numeric values, allowing comparison of arbitrarily large numbers without overflow risk
- **Leading zeros handling**: Numbers with leading zeros (e.g., "01") are considered larger than the same number without (e.g., "1")
- **Full UTF-8 support**: Correctly handles all UTF-8 encoded characters (1-byte, 2-byte, 3-byte, and 4-byte sequences)
- **Robust implementation**: Properly handles invalid UTF-8 sequences and edge cases


## Installation

```bash
luarocks install string-natcmp
```

## Usage

**Basic Comparison**

```lua
local natcmp = require('string.natcmp')

-- Compare two strings in natural order
print(natcmp('file10.txt', 'file2.txt')) -- 1 (file10.txt > file2.txt)
print(natcmp('file2.txt', 'file10.txt')) -- -1 (file2.txt < file10.txt)
print(natcmp('file10.txt', 'file10.txt')) -- 0 (equal)

-- Compare strings with leading zeros
print(natcmp('file01.txt', 'file1.txt')) -- 1 (file01.txt > file1.txt)
```


**UTF-8 comparison**

```lua
local natcmp = require('string.natcmp')

-- UTF-8 comparison using the utf8 submodule
print(natcmp.utf8('あ', 'い')) -- -1 ('あ' < 'い')
print(natcmp.utf8('file1-あ', 'file1-い')) -- -1 ('file1-あ' < 'file1-い')

-- Mixed UTF-8 and numerics
print(natcmp.utf8('あ10', 'あ2')) -- 1 ('あ10' > 'あ2')
```


**Sorting Tables**

```lua
local dump = require('dump')
local natcmp = require('string.natcmp')
local files = {
    'file10.txt',
    'file1.txt',
    'file2.txt',
    'file01.txt',
}

-- Sort using the provided less-than comparator
table.sort(files, natcmp.lt)
print(dump(files))
-- {
--     [1] = "file1.txt",
--     [2] = "file01.txt",
--     [3] = "file2.txt",
--     [4] = "file10.txt"
-- }

-- Real-world example with mixed content
local mixed = {
    'README.md',
    'example-10.txt',
    'example-1.txt',
    'image001.jpg',
    'image1.jpg',
    'image10.jpg',
    'い10.txt',
    'あ01.txt',
    'あ1.txt',
}

table.sort(mixed, natcmp.utf8.lt)
print(dump(mixed))
-- {
--     [1] = "example-1.txt",
--     [2] = "example-10.txt",
--     [3] = "image1.jpg",
--     [4] = "image001.jpg",
--     [5] = "image10.jpg",
--     [6] = "README.md",
--     [7] = "あ1.txt",
--     [8] = "あ01.txt",
--     [9] = "い10.txt"
-- }
```

## res = natcmp(a, b)

Compares two strings in natural order using ASCII comparison for non-digit parts.

**Parameters:**

- `a:string`: a left-hand side string to compare.
- `b:string`: a right-hand side string to compare.

**Returns:**

- `res:integer`: a result of comparison.
  - `-1`: if a < b
  - `0`: if a == b
  - `1`: if a > b


## res = natcmp.lt(a, b)

Less-than comparator function compatible with `table.sort()`.

**Parameters:**

- `a:string`: a left-hand side string to compare.
- `b:string`: a right-hand side string to compare.

**Returns:**

- `res:boolean`: `true` if `a` < `b`, otherwise `false`.


## res = natcmp.utf8(a, b)

Compares two strings in natural order with full UTF-8 support.

**Parameters:**

- `a:string`: a left-hand side string to compare.
- `b:string`: a right-hand side string to compare.

**Returns:**

- `res:integer`: a result of comparison.
    - `-1`: if a < b
    - `0`: if a == b
    - `1`: if a > b


## res = natcmp.utf8.lt(a, b)

UTF-8 aware less-than comparator function compatible with `table.sort()`.

**Parameters:**

- `a:string`: a left-hand side string to compare.
- `b:string`: a right-hand side string to compare.

**Returns:**

- `res:boolean`: `true` if `a` < `b`, otherwise `false`.


## Implementation Details

- Comparison algorithm: The library splits strings into alphanumeric segments and compares them accordingly
- Number handling: Numeric parts are compared digit by digit
- Leading zeros: Numbers with the same value but different digit counts are compared based on digit count
- UTF-8 processing: UTF-8 characters are correctly decoded to code points before comparison
- Error handling: Invalid UTF-8 sequences are replaced with U+FFFD (replacement character)


## License

MIT License - Copyright (C) 2025 Masatoshi Fukunaga 
