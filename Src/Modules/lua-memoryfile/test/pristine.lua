require "memoryfile-test"

module("test.pristine", lunit.testcase, package.seeall)

function test_no_global_clobbering ()
    local globals = {}
    for key in pairs(_G) do globals[key] = true end

    local lib = require "memoryfile"

    for key in pairs(_G) do
        lunit.assert_not_nil(globals[key],
                             "global '" .. key .. "' created by lib")
    end
    for key in pairs(globals) do
        lunit.assert_not_nil(_G[key],
                             "global '" .. key .. "' destroyed by lib")
    end
end

-- vi:ts=4 sw=4 expandtab
