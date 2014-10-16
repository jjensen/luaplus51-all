require "memoryfile-test"
local MemFile = require "memoryfile"

module("test.io", lunit.testcase, package.seeall)

function test_module_metadata ()
    is("string", type(MemFile._VERSION))
    assert(MemFile._VERSION:len() > 0)
    is("memoryfile", MemFile._NAME)
end

function test_read_small ()
    local f = MemFile.open("frobnitz", "r")
    is("frobnitz", tostring(f))
    is(0, f:seek())
    is(8, f:size())
    is("frob", f:read(4))
    is(4, f:seek())
    is(8, f:size())
    is("nitz", f:read("*a"))
    is(8, f:seek())
    is(8, f:size())
    is("frobnitz", tostring(f))
end

function test_read_all ()
    local f = MemFile.open("frobnitz", "r")
    is("frobnitz", f:read("*a"))
    local all2, all3 = f:read("*a", "*all")
    is("", all2)
    is("", all3)
    is(8, f:seek())
    is(8, f:size())
    is("frobnitz", tostring(f))
end

function test_read_lines ()
    local f = MemFile.open("foo\nbar\nbaz\r\n", "r")
    is("foo", f:read("*l"))
    is(4, f:seek())
    local line2, line3 = f:read("*l", "*line")
    is("bar", line2)
    is("baz\r", line3)
    is(nil, f:read("*l"))
    is(13, f:seek())
    is(13, f:size())
    is("foo\nbar\nbaz\r\n", tostring(f))

    f = MemFile.open("foo", "r")
    is("foo", f:read("*l"))
    is(nil, f:read("*l"))

    f = MemFile.open("foo\nbar", "r")
    is("foo", f:read("*l"))
    is("bar", f:read("*l"))
    is(nil, f:read("*l"))

    f = MemFile.open("", "r")
    is(nil, f:read("*l"))
end

function test_read_lines_iter ()
    local f = MemFile.open("foo\nbar\nbaz\r\n", "r")
    local t = {}
    for line in f:lines() do t[#t + 1] = line end
    is(3, #t)
    is("foo", t[1])
    is("bar", t[2])
    is("baz\r", t[3])

    f = MemFile.open("", "r")
    for line in f:lines() do assert_fail("wasn't expecting line") end
end

function test_default_mode ()
    local f = MemFile.open("frobnitz")
    is("frobnitz", f:read("*a"))
    assert_nil(f:read(0))
    is("frobnitz", tostring(f))
end

function test_extra_mode_chars ()
    local f = MemFile.open("frobnitz", "r+bx")
    is("frobnitz", f:read("*a"))
    assert_nil(f:read(0))
    is("frobnitz", tostring(f))
end

function test_write_mode ()
    local f = MemFile.open("foo", "w")
    is(0, f:size())
    is(0, f:seek())
    is("", tostring(f))
end

function test_write ()
    local f = MemFile.open(nil, "w")
    is(0, f:size())
    is(0, f:seek())
    is("", tostring(f))
    assert(f:write("foobar", "baz", 23.25))
    is(14, f:size())
    is(14, f:seek())
    is("foobarbaz23.25", tostring(f))

    is(6, f:seek("set", 6))
    assert(f:write("xyzzy"))
    is(14, f:size())
    is(11, f:seek())
    is(".25", f:read("*a"))
    is("foobarxyzzy.25", tostring(f))

    is(6, f:seek("set", 6))
    assert(f:write("fol-de-rol-de-ra"))
    is(22, f:size())
    is(22, f:seek())
    is("foobarfol-de-rol-de-ra", tostring(f))
end

function test_append ()
    local f = MemFile.open("", "a")
    assert(f:write("foo"))
    assert(f:write("barb"))
    is("foobarb", tostring(f))
    is(7, f:size())
    is(0, f:seek())
    is("foobar", f:read(6))
    is(7, f:size())
    is(6, f:seek())
    assert(f:write("quuux"))
    is(12, f:size())
    is(6, f:seek())
    is("bquuux", f:read("*a"))

    f = MemFile.open("foobar", "a")
    is(6, f:size())
    is(0, f:seek())
    assert(f:write("\nquux"))
    is(11, f:size())
    is(0, f:seek())
    is("foobar", f:read("*l"))
    is(11, f:size())
    is(7, f:seek())
    is("quux", f:read("*a"))
end

function test_seek ()
    local f = MemFile.open("foobar")
    is(0, f:seek())
    is(0, f:seek("cur"))
    is(0, f:seek("cur", 0))
    is(0, f:seek())

    is(0, f:seek("set"))
    is(0, f:seek("set", 0))
    is(0, f:seek())

    is(3, f:seek("set", 3))
    is("b", f:read(1))

    assert(f:seek("set", 3))
    is(3, f:seek("cur", 0))
    is("b", f:read(1))

    assert(f:seek("set", 3))
    is(1, f:seek("cur", -2))
    is("oo", f:read(2))

    assert(f:seek("set", 3))
    is(5, f:seek("cur", 2))
    is("r", f:read(2))

    assert(f:seek("set", 3))
    is(6, f:seek("end", 0))
    assert_nil(f:read(0))

    assert(f:seek("set", 3))
    is(4, f:seek("end", -2))
    is("ar", f:read(2))
end

function test_size ()
    local f = MemFile.open("frob\0nitz")
    is(9, f:size())
    is(9, f:size(999))
    is(999, f:size())
    is("frob\0nitz" .. ("\0"):rep(990), tostring(f))
    is(999, f:size(6))
    is(6, f:size())
    is("frob\0n", tostring(f))
    is(6, f:size(0))
    is(0, f:size())
    is("", tostring(f))
    is(0, f:size(1000))
    is(1000, f:size())
    is(("\0"):rep(1000), tostring(f))
end

function test_noops ()
    local f = MemFile.open("foobar")
    assert_true(f:flush())
    assert_true(f:setvbuf())
    is("foobar", tostring(f))
end

function test_close ()
    local f = MemFile.open("foobar")
    assert_true(f:close())
    is("", tostring(f))
    is(0, f:size())
    f:write("new text")
    is("new text", tostring(f))
    is(8, f:size())
    assert_true(f:close())  -- make sure it's repeatable
    is("", tostring(f))
    is(0, f:size())
end

function test_open_bad_usage ()
    assert_error("too many args", function () MemFile.open("", "r", nil) end)
    assert_error("bad type of input data",
                 function () MemFile.open(true, "r") end)
    assert_error("bad type of mode", function () MemFile.open("", true) end)
    assert_error("bad mode letter", function () MemFile.open("", "x") end)
end

function test_read_bad_usage ()
    local f = MemFile.open("foo", "r")
    assert_error("invalid option, no asterisk", function () f:read("x") end)
    assert_error("invalid option, unknown format", function () f:read("*x") end)
end

local function test_io_error (patn, ok, err)
    assert_nil(ok)
    assert_not_nil(err)
    assert((err:find(patn)))
end

function test_seek_bad_usage ()
    local f = MemFile.open("foo", "r")
    assert_error("bad whence type", function () f:seek(123, 1) end)
    assert_error("bad whence value", function () f:seek("foo", 1) end)
    assert_error("bad offset type", function () f:seek("set", true) end)
end

function test_seek_out_of_range ()
    local f = MemFile.open("foo", "r")
    f:seek("set", 1)
    test_io_error("before start", f:seek("set", -1))
    is(1, f:seek())
    test_io_error("after end", f:seek("set", 4))
    is(1, f:seek())
    test_io_error("before start", f:seek("cur", -2))
    is(1, f:seek())
    test_io_error("after end", f:seek("cur", 3))
    is(1, f:seek())
    test_io_error("before start", f:seek("end", -4))
    is(1, f:seek())
    test_io_error("after end", f:seek("end", 1))
    is(1, f:seek())
end

function test_write_bad_usage ()
    local f = MemFile.open(nil, "w")
    assert_error("first arg not string or number",
                 function () f:write(true) end)
    assert_error("second arg not string or number",
                 function () f:write("foo", true) end)
end

function test_size_bad_usage ()
    local f = MemFile.open("foo", "r")
    assert_error("new size not number", function () f:size(true) end)
    assert_error("new size negative", function () f:size(-1) end)
end

function test_grow_buffer ()
    local f = MemFile.open()
    for i = 1, 100000 do f:write("foobar\n") end
    is(("foobar\n"):rep(100000), tostring(f))
    f:size(0)
    is("", tostring(f))
end

-- vi:ts=4 sw=4 expandtab
