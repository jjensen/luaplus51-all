-- This will load the new copy of the library on Unix systems where it's built
-- with libtool.
package.cpath = ".libs/liblua-?.so;" .. package.cpath

require "lunit"

is = lunit.assert_equal

-- vi:ts=4 sw=4 expandtab
