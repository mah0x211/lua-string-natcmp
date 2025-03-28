local assert = require('assert')
local natcmp = require('string.natcmp')
local alltests = {}
local testcase = setmetatable({}, {
    __newindex = function(_, k, v)
        assert(not alltests[k], 'duplicate test name: ' .. k)
        alltests[k] = true
        alltests[#alltests + 1] = {
            name = k,
            func = v,
        }
    end,
})

function testcase.natcmp_basic()
    -- test that return 0 if two strings are equal
    local res = natcmp('foo', 'foo')
    assert.equal(res, 0)

    -- test that return -1 if first string is less than second string
    res = natcmp('a', 'b')
    assert.equal(res, -1)

    -- test that return 1 if first string is greater than second string
    res = natcmp('b', 'a')
    assert.equal(res, 1)

    -- test empty strings
    res = natcmp('', '')
    assert.equal(res, 0)

    -- test one empty string
    res = natcmp('', 'a')
    assert.equal(res, -1)
    res = natcmp('a', '')
    assert.equal(res, 1)
end

function testcase.natcmp_numeric()
    -- test that compare strings with numbers
    local res = natcmp('foo1', 'foo2')
    assert.equal(res, -1)

    -- test numbers at the beginning
    res = natcmp('1foo', '2foo')
    assert.equal(res, -1)

    -- test natural sorting with different digit lengths
    res = natcmp('file1', 'file10')
    assert.equal(res, -1)
    res = natcmp('file10', 'file2')
    assert.equal(res, 1)

    -- test large numbers
    res = natcmp('file999999999', 'file1000000000')
    assert.equal(res, -1)

    -- test leading zeros (same numeric value, different digit count)
    res = natcmp('file01', 'file1')
    assert.equal(res, 1) -- '01' is considered greater than '1' in natural sort
    res = natcmp('file1', 'file01')
    assert.equal(res, -1)

    -- test multiple number groups
    res = natcmp('file1-2', 'file1-10')
    assert.equal(res, -1)
end

function testcase.natcmp_mixed()
    -- test mixed alphanumeric comparison
    local res = natcmp('a1b2', 'a1b10')
    assert.equal(res, -1)

    -- test when one string has more segments
    res = natcmp('a1b2', 'a1b2c3')
    assert.equal(res, -1)
    res = natcmp('a1b2c3', 'a1b2')
    assert.equal(res, 1)

    -- test digit vs non-digit
    res = natcmp('a1', 'ab')
    assert.equal(res, -1) -- digits are less than non-digits
    res = natcmp('ab', 'a1')
    assert.equal(res, 1)

    -- test number is less than string
    res = natcmp.utf8('a1', 'ab')
    assert.equal(res, -1) -- 'a1' < 'ab'
end

function testcase.natcmp_utf8()
    -- test UTF-8 characters
    local res = natcmp('あ', 'い')
    assert.equal(res, -1)
    res = natcmp('い', 'あ')
    assert.equal(res, 1)

    -- test UTF-8 with numbers
    res = natcmp('あ1', 'あ2')
    assert.equal(res, -1)
    res = natcmp('あ10', 'あ2')
    assert.equal(res, 1)

    -- test case insensitivity for ASCII
    res = natcmp('A', 'a')
    assert.equal(res, 0)
    res = natcmp('aB', 'Ab')
    assert.equal(res, 0)

    -- test mixed ASCII and UTF-8
    res = natcmp('a漢字', 'b漢字')
    assert.equal(res, -1)
end

function testcase.natcmp_lt()
    -- test that return true if first string is less than second string
    local res = natcmp.lt('a', 'b')
    assert.equal(res, true)

    -- test that return false if first string is greater than second string
    res = natcmp.lt('b', 'a')
    assert.equal(res, false)

    -- test that return false if two strings are equal
    res = natcmp.lt('foo', 'foo')
    assert.equal(res, false)

    -- test with numbers
    res = natcmp.lt('file1', 'file2')
    assert.equal(res, true)
    res = natcmp.lt('file2', 'file10')
    assert.equal(res, true)

    -- test with UTF-8
    res = natcmp.lt('あ', 'い')
    assert.equal(res, true)
end

function testcase.natcmp_utf8_multibyte()
    -- test 2-byte UTF-8 characters
    local res = natcmp('é', 'ñ') -- é (0xC3 0xA9) < ñ (0xC3 0xB1)
    assert.equal(res, -1)

    -- test 3-byte UTF-8 characters (already in original test)
    res = natcmp('あ', 'い') -- Japanese hiragana
    assert.equal(res, -1)

    -- test 4-byte UTF-8 characters (Emoji)
    res = natcmp('😀', '😁') -- GRINNING FACE < GRINNING FACE WITH SMILING EYES
    assert.equal(res, -1)
    res = natcmp('text😀', 'text😁')
    assert.equal(res, -1)

    -- test mixing byte lengths
    res = natcmp('a', 'é') -- 1-byte < 2-byte
    assert.equal(res, -1)
    res = natcmp('é', 'あ') -- 2-byte < 3-byte
    assert.equal(res, -1)
    res = natcmp('あ', '😀') -- 3-byte < 4-byte
    assert.equal(res, -1)

    -- test with numbers
    res = natcmp('é1', 'é2')
    assert.equal(res, -1)
    res = natcmp('😀1', '😀2')
    assert.equal(res, -1)
    res = natcmp('😀10', '😀2')
    assert.equal(res, 1) -- natural sort logic applies to emoji + number too
end

function testcase.natcmp_invalid_utf8()
    -- test invalid UTF-8 sequences
    -- Single continuation byte
    local invalid1 = string.char(0x80)
    local invalid2 = string.char(0x81)
    local res = natcmp.utf8(invalid1, invalid2)
    assert.equal(res, 0) -- treats as compare as 0xFFFD < 0xFFFD

    -- Incomplete 2-byte sequence
    invalid1 = string.char(0xC0) -- Missing continuation byte
    invalid2 = string.char(0xC1)
    res = natcmp.utf8(invalid1, invalid2)
    assert.equal(res, 0)

    -- Incomplete 3-byte sequence
    invalid1 = string.char(0xE0) .. string.char(0xA0) -- Missing one continuation byte
    invalid2 = string.char(0xE0) .. string.char(0xA1)
    res = natcmp.utf8(invalid1, invalid2)
    assert.equal(res, 0)

    -- Incomplete 4-byte sequence
    invalid1 = string.char(0xF0) .. string.char(0x90) .. string.char(0x80) -- Missing one continuation byte
    invalid2 = string.char(0xF0) .. string.char(0x90) .. string.char(0x81)
    res = natcmp.utf8(invalid1, invalid2)
    assert.equal(res, 0)

    -- Compare valid and invalid sequences
    res = natcmp.utf8('a', invalid1)
    assert.equal(res, -1) -- 'a' < invalid sequence (treated as 0xFFFD)

    -- Test that the library doesn't crash with invalid sequences
    res = natcmp.utf8(invalid1, 'a')
    assert.equal(res, 1) -- invalid sequence > 'a' (treated as 0xFFFD)

    -- Mix of valid and invalid
    res = natcmp.utf8('a' .. invalid1, 'a' .. invalid2)
    assert.equal(res, 0)
end

function testcase.natcmp_edge_cases()
    -- test with leading, trailing and multiple spaces
    local res = natcmp('  a', 'a')
    assert.equal(res, -1) -- space is less than 'a'
    res = natcmp('a  ', 'a')
    assert.equal(res, 1)

    -- test with various special characters
    res = natcmp('a-1', 'a-2')
    assert.equal(res, -1)
    res = natcmp('a_1', 'a_2')
    assert.equal(res, -1)

    -- test very long strings
    local long1 = string.rep('a', 1000) .. '1'
    local long2 = string.rep('a', 1000) .. '2'
    res = natcmp(long1, long2)
    assert.equal(res, -1)
end

-- Helper function for table sort tests
local function test_sort(cmpfn, input, expected, msg)
    local original = {}
    for i, v in ipairs(input) do
        original[i] = v
    end

    table.sort(input, cmpfn)

    for i = 1, #expected do
        assert.equal(input[i], expected[i], string.format(
                         "%s: Mismatch at position %d: expected '%s', got '%s' (original: %s)",
                         msg or "Sort failed", i, expected[i], input[i],
                         table.concat(original, ", ")))
    end
end

function testcase.natcmp_table_sort_basic()
    -- Basic alphabetical sorting
    local basic = {
        "b",
        "c",
        "a",
    }
    test_sort(natcmp.lt, basic, {
        "a",
        "b",
        "c",
    }, "Basic alphabet sort")

    -- Check for stability with equal elements
    local stability = {
        "a1",
        "b1",
        "a1",
        "c1",
    }
    local expected_stability = {
        "a1",
        "a1",
        "b1",
        "c1",
    }
    test_sort(natcmp.lt, stability, expected_stability, "Sort stability test")
end

function testcase.natcmp_table_sort_numeric()
    -- Natural sort with numbers
    local files = {
        "file10.txt",
        "file1.txt",
        "file2.txt",
        "file20.txt",
    }
    test_sort(natcmp.lt, files, {
        "file1.txt",
        "file2.txt",
        "file10.txt",
        "file20.txt",
    }, "Natural file sort")

    -- Leading zeros handling
    local zeros = {
        "file01",
        "file1",
        "file001",
        "file10",
    }
    test_sort(natcmp.lt, zeros, {
        "file1",
        "file01",
        "file001",
        "file10",
    }, "Leading zeros sort")

    -- Multiple number groups
    local multiGroup = {
        "file1-10",
        "file1-2",
        "file10-1",
        "file2-1",
    }
    test_sort(natcmp.lt, multiGroup, {
        "file1-2",
        "file1-10",
        "file2-1",
        "file10-1",
    }, "Multiple number groups sort")
end

function testcase.natcmp_table_sort_utf8()
    -- UTF-8 character sorting
    local utf8 = {
        "お",
        "え",
        "う",
        "い",
        "あ",
        "き",
        "か",
    }
    test_sort(natcmp.utf8.lt, utf8, {
        "あ",
        "い",
        "う",
        "え",
        "お",
        "か",
        "き",
    }, "UTF-8 sort")

    -- Mixed case (ASCII, numbers, UTF-8)
    local mixed = {
        "z",
        "あ10",
        "あ2",
        "a10",
        "a2",
        "b",
        "c",
        "file01",
        "file1",
    }
    test_sort(natcmp.utf8.lt, mixed, {
        "a2",
        "a10",
        "b",
        "c",
        "file1",
        "file01",
        "z",
        "あ2",
        "あ10",
    }, "Mixed content sort")
end

function testcase.natcmp_table_sort_invalid()
    -- Invalid UTF-8 sequences
    local invalid1 = string.char(0x80) -- Single continuation byte
    local invalid2 = string.char(0xE0) .. string.char(0xA0) -- Incomplete 3-byte sequence

    local invalid = {
        "z",
        invalid1,
        "a",
        invalid2,
    }
    local sorted_invalid = {
        "a",
        "z",
        invalid1,
        invalid2,
    }
    test_sort(natcmp.utf8.lt, invalid, sorted_invalid, "Invalid UTF-8 sort")
end

function testcase.natcmp_table_sort_realistic()
    -- Real-world file naming pattern test
    local realistic = {
        "README.md",
        "example-10.txt",
        "example-1.txt",
        "image001.jpg",
        "image1.jpg",
        "image10.jpg",
        "日本語01.txt",
        "日本語1.txt",
        "日本語10.txt",
    }

    local expected_realistic = {
        "example-1.txt",
        "example-10.txt",
        "image1.jpg",
        "image001.jpg",
        "image10.jpg",
        "README.md",
        "日本語1.txt",
        "日本語01.txt",
        "日本語10.txt",
    }

    test_sort(natcmp.utf8.lt, realistic, expected_realistic,
              "Real-world filenames sort")
end

function testcase.natcmp_table_sort_performance()
    -- Performance test with larger array
    local large = {}
    for i = 1, 100 do
        large[i] = "item" .. (101 - i) -- Reverse order
    end

    local expected_large = {}
    for i = 1, 100 do
        expected_large[i] = "item" .. i
    end

    test_sort(natcmp.lt, large, expected_large, "Large array sort")

    -- Very large numbers
    local largeNums = {
        "file9999999999",
        "file10000000000",
        "file999999999",
        "file1000000000",
    }
    test_sort(natcmp.lt, largeNums, {
        "file999999999",
        "file1000000000",
        "file9999999999",
        "file10000000000",
    }, "Large numbers sort")
end

local gettime = os.time
local stdout = io.stdout
local elapsed = gettime()
local errs = {}
print(string.format('Running %d tests...\n', #alltests))
for _, test in ipairs(alltests) do
    stdout:write('- ', test.name, ' ... ')
    local t = gettime()
    local ok, err = pcall(test.func)
    t = gettime() - t
    if ok then
        stdout:write('ok')
    else
        stdout:write('failed')
        errs[#errs + 1] = {
            name = test.name,
            err = err,
        }
    end
    stdout:write(' (', string.format('%.2f', t), ' sec)\n')
end
elapsed = gettime() - elapsed
print('')
if #errs == 0 then
    print(string.format('%d tests passed. (%.2f sec)\n', #alltests, elapsed))
    os.exit(0)
end

print(string.format('Failed %d tests:\n', #errs))
local stderr = io.stderr
for _, err in ipairs(errs) do
    stderr:write('- ', err.name)
    stderr:write(err.err, '\n')
end
print('')
os.exit(-1)
