## Overview

*prettydump* is a Lua module for pretty printing Lua values, especially tables.  All data types are handled, though some write themselves out as comments (such as functions, userdata, and threads).

*prettydump* is derived from former LuaPlus versions that had built-in pretty printing support.  In the LuaPlus nextgen branch, that pretty printing support has been largely removed.

Depending on setup (whether LuaPlus is the driving engine), the *prettydump* module goes to great lengths to not modify any of the internal Lua state.


## Example

<pre>
    local prettydump = require 'prettydump'

    local myTable = {
        Name = 'Joe',
        HitPoints = 100,
    }

    prettydump.dumpascii('output.lua', 'myTable', myTable)
</pre>


## Reference Manual

This is a reference of all of the *prettydump* module's methods.

## Module `prettydump`

**result = prettydump.dumpascii(whereToWrite, key, value, alphabetical, indentLevel, maxIndentLevel, writeAll)**

* `whereToWrite` - The name of the file to write to disk or the string `":string"` to return the pretty printed value dump as a string.
* `key` - The key name of the initial Lua table to write to disk.  This is usually a string, but it could be a number, too.  If the key is `nil`, no key is written.
* `value` - The value to write.
* `alphabetical` - If `true`, each table's keys are sorted.  (Optional: Defaults to true.)
* `indentLevel` - The number of tabs to indent each line.  (Optional: Defaults to 0.)
* `maxIndentLevel` - The maximum number of nested tables allowed in the write.  If this value is exceeded, then no carriage returns are inserted.  (Optional: Defaults to 0xFFFFFFFF.)
* `writeAll` - If true, writes all Lua objects out, including function and user data information. (Optional: Defaults to false.)


**result = prettydump.format(formatstring, ...)**

(text largely derived from the Lua manual)

Returns a formatted version of its variable number of arguments following the description given in its first argument (which must be a string). The format string follows the same rules as the `printf` family of standard C functions. The only differences are that the options/modifiers `*`, `l`, `L`, `n`, `p`, and `h` are not supported and that there are two extra options, `q` and `Q`. The q option formats a string in a form suitable to be safely read back by the Lua interpreter: the string is written between double quotes, and all double quotes, newlines, embedded zeros, and backslashes in the string are correctly escaped when written. The `Q` option is similar, but it writes strings in a different form with more escapes.

The options `c`, `d`, `E`, `e`, `f`, `g`, `G`, `i`, `o`, `u`, `X`, and `x` all expect a number as argument, whereas `q`, `Q`, and `s` expect a string.

This function does not accept string values containing embedded zeros.


## License

The *prettydump* module is licensed under the terms of the MIT license and can be used unrestricted in any place where Lua could be used.

===============================================================================

Lua License
-----------

Lua is licensed under the terms of the MIT license reproduced below.
This means that Lua is free software and can be used for both academic
and commercial purposes at absolutely no cost.

For details and rationale, see http://www.lua.org/license.html .

===============================================================================

Copyright (C) 1994-2012 Lua.org, PUC-Rio.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

===============================================================================

(end of COPYRIGHT)

