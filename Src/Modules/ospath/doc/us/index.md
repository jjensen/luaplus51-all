# Introduction





# Credits

Portions of this module come from <http://lua-users.org/wiki/ExtensionProposal>.





# ospath API

Note that all these functions return the standard (nil,"error message") on failure and that, unless otherwise specified, they return (true) on success.






-------------------------



## File System

### ospath.access(path [ , mode ])

Determines if the file specified by `path` is read-only or not.  Returns

`mode` is optional or can be one of the following:

* If not specified, the file is checked for existence only.
* `w` - Checks whether the file is write-only.
* `r` - Checks whether the file is read-only.
* `rw` - Checks whether the file is readable and writeable.

Returns true if the file matches the mode, false otherwise.

<pre>
    print(ospath.access('myfile.txt'))      -- Prints true if myfile.txt exists.
    print(ospath.access('myfile.txt', 'w')  -- Prints true if myfile.txt is write-only.
    print(ospath.access('myfile.txt', 'r')  -- Prints true if myfile.txt is read-only.
    print(ospath.access('myfile.txt', 'rw') -- Prints true if myfile.txt is readable and writeable.
</pre>




### ospath.chdir(pathname)

Change the working directory to `pathname`.

<pre>
    ospath.chdir(os.getenv("HOME"))
</pre>




### ospath.chmod(pathname, mode)

Changes the file `mode` of `pathname`.

`mode` may be one of the following bitfields:

* `400` - Allow read by owner.
* `200` - Allow write by owner.
* `100` - Allow execution by owner.
* `040` - Allow read by group members.
* `020` - Allow write by group members.
* `010` - Allow execution by group members.
* `004` - Allow read by others.
* `002` - Allow write by others.
* `001` - Allow execution by others.

`mode` may also be symbolic:

* `w` - Writable.
* `r` - Readable.
* `rw` - Both read and writable.

<pre>
    ospath.chmod('script', 777)

    ospath.chmod('writablefile', 'w')
</pre>




### cwd = ospath.getcwd()

Retrieves the current working directory.

<pre>
    cwd = ospath.getcwd()
</pre>




### ospath.hardlink(sourceFilename, destinationFilename)

Creates a hardlink named `destinationFilename` that points to `sourceFilename`.

<pre>
    ospath.hardlink('a', 'b')               -- Hardlinks new file entry 'b' to existing file entry 'a'
</pre>




### ospath.mkdir(pathname)

Create the directory described by `pathname`.  `pathname` is in the form of `dirA/dirB/dirC/` or `dirA/dirB/dirC/filename`.  All directories up to the final slash (or backslash) are created.  The filename component is ignored.

<pre>
    dirName = 'dirA/dirB/dirC'
    ospath.mkdir(dirName)                    -- Creates dirA/dirB/
    dirName = 'dirA/dirB/dirC/'
    ospath.mkdir(dirName)                    -- Creates dirA/dirB/dirC/
    filename = 'dirA/dirB/dirC/filename'
    ospath.mkdir(filename)                   -- Creates dirA/dirB/dirC/
</pre>




### ospath.remove(pathname)

Removes the file or directory at `pathname`.  If `pathname` is a directory, all child directories and files within those directories are destroyed.

<pre>
    ospath.remove('dirA/dirB/')              -- Remove dirB/ recursively
    ospath.remove('dirA/file.txt')           -- Remove file.txt from dirA/
</pre>




### ospath.symboliclink(symlinkFilename, targetFilename [, isDirectory = false])

Creates a symbolic link named `symlinkFilename` that points to `targetFilename`.  If `isDirectory` is specified, a directory-level symbolic link is created.

<pre>
    ospath.symboliclink('a', 'b')               -- Symbolically links new file entry 'a' to existing file entry 'b'
</pre>




### ospath.copy_file(srcfilename, destfilename)

Copies the file named `srcfilename` to `destfilename` preserving file attributes and timestamp.

<pre>
    ospath.copy_file('filea.txt', 'fileb.txt')    -- Copy filea.txt to fileb.txt
</pre>





### ospath.copy_directory(sourceDirectory, destinationDirectory [, options])

Copies the directory named `srcdirectory` into `destdirectory` preserving file attributes and timestamps.  This function differs from `ospath.mirrordirectory` in that the `srcdirectory` files and directories are overlayed onto `destdirectory`.  `ospath.mirrordirectory` removes extra files and directories.

`options` is an optional table containing one or more of the following members:

* `callback` - A function in the following form: `function(operation, filenameA, filenameB)`
    * `function('copy', sourceFilename, destinationFilename)`
    * `function('del', destinationFilename)`
* `noop` - (defaults to `false`) If set to `true`, a directory scan is performed, but no copies or deletes take place.
* `deleteExtra` - (defaults to `false`) If set to `true`, files existing in the `destinationDirectory` that do not exist in the `sourceDirectory` are removed from the `destinationDirectory`.
* `hardlink` - (defaults to `false`) If set to `true`, files needing to be copied to the `destinationDirectory` are hardlinked against those in the `sourceDirectory`.
* `copyfile` - A function in the following form: `function(sourceFilename, destinationFilename)`

<pre>
    ospath.copy_directory('/dira', '/dirb')
    ospath.copy_directory('/dira', '/dirb', { callback = print })
    ospath.copy_directory('/dira', '/dirb', { callback = print, noop = true })
</pre>





### ospath.mirror_directory(sourceDirectory, destinationDirectory, options)

Mirrors the directory named `srcdirectory` to `destdirectory` preserving file attributes and timestamps.  `ospath.mirror_directory` removes extra files and directories.

`ospath.mirror_directory` is identical to `ospath.copy_directory` except for `options.deleteExtra = true` being provided automatically.

<pre>
    ospath.mirror_directory('/dira', '/dirb')
</pre>





### ospath.move_file(srcfilename, destfilename)

Moves the file named `srcfilename` to `destfilename`.

<pre>
    ospath.move_file('filea.txt', 'fileb.txt')    -- Move filea.txt to fileb.txt
</pre>







--------------

## I/O (locking and pipes)

### ospath.lock(file, mode, offset, length)

Lock or unlock a file or a portion of a file; 'mode' is either "r" or "w" or "u"; 'offset' and 'length' are optional.  A mode of "r" requests a read lock, "w" requests a write lock, and "u" releases the lock. Note that the locks may be either advisory or mandatory.





### ospath.unlock(file, offset, length)

Equivalent to `ospath.lock(file, "u", offset, length)`.






## Path Manipulation

### ospath.add_extension(path, extension)

Adds `extension` to the end of `path` even if one already exists.

Returns the new ospath.

<pre>
    assert(ospath.add_extension('', 'ext') == '.ext')
    assert(ospath.add_extension(' ', ' ext') == ' . ext')
    assert(ospath.add_extension('', '.ext') == '.ext')
    assert(ospath.add_extension('hello', 'ext') == 'hello.ext')
    assert(ospath.add_extension('hello', '.ext') == 'hello.ext')
    assert(ospath.add_extension('hello.txt', 'ext') == 'hello.txt.ext')
    assert(ospath.add_extension('hello.txt', '.ext') == 'hello.txt.ext')
</pre>





### ospath.add_slash(path)

Adds a slash to the end of `path` if it doesn't already exist.

Returns the new ospath.

<pre>
    assert(ospath.add_slash('') == '/')
    assert(ospath.add_slash(' ') == ' /')
    assert(ospath.add_slash('hello') == 'hello/')
    assert(ospath.add_slash(' hello') == ' hello/')
    assert(ospath.add_slash(' hello ') == ' hello /')
    assert(ospath.add_slash('hello/') == 'hello/')
</pre>





### ospath.append(leftPath, rightPath)

Appends `leftPath` and `rightPath` together, adding a slash between the two path components.  If `rightPath` is an absolute path, it is not appended to `leftPath` and is returned directly.

Returns the new ospath.

<pre>
    assert(ospath.append('', 'filename.txt') == 'filename.txt')
    assert(ospath.append('', 'dir', 'filename.txt') == 'dir/filename.txt')
    assert(ospath.append('', 'dirA', '', 'dirB', 'filename.txt') == 'dirA/dirB/filename.txt')
    assert(ospath.append('..', 'filename.txt') == '../filename.txt')
    assert(ospath.append('root', 'filename.txt') == 'root/filename.txt')
    assert(ospath.append('root', 'dir', 'filename.txt') == 'root/dir/filename.txt')
    assert(ospath.append('root', 'dirA', '', 'dirB', 'filename.txt') == 'root/dirA/dirB/filename.txt')
    assert(ospath.append('root', 'dirA', '', 'dirB', '..', 'filename.txt') == 'root/dirA/dirB/../filename.txt')
    assert(ospath.append('root', 'dirA', '', '/dirB', '..', 'filename.txt') == '/dirB/../filename.txt')
</pre>





### ospath.combine(leftPath, rightPath)
### ospath.join(leftPath, rightPath)

Combines `leftPath` and `rightPath`, adding a slash between the two path components and simplifying the path by collapsing `.` and `..` directories.  If `rightPath` is an absolute path, it is not appended to `leftPath` and is returned directly.

Returns the new ospath.

<pre>
    assert(ospath.combine('', 'filename.txt') == 'filename.txt')
    assert(ospath.combine('', 'dir', 'filename.txt') == 'dir/filename.txt')
    assert(ospath.combine('', 'dirA', '', 'dirB', 'filename.txt') == 'dirA/dirB/filename.txt')
    assert(ospath.combine('..', 'filename.txt') == '../filename.txt')
    assert(ospath.combine('root', 'filename.txt') == 'root/filename.txt')
    assert(ospath.combine('root', 'dir', 'filename.txt') == 'root/dir/filename.txt')
    assert(ospath.combine('root', 'dirA', '', 'dirB', 'filename.txt') == 'root/dirA/dirB/filename.txt')
    assert(ospath.combine('root', 'dirA', '', 'dirB', '..', 'filename.txt') == 'root/dirA/filename.txt')
</pre>





### ospath.escape(path)

Properly escapes and quotes `path`, if needed.

Returns the new ospath.

<pre>
    assert(ospath.escape('') == '')
    assert(ospath.escape(' ') == '" "')
    assert(ospath.escape('filename.txt') == 'filename.txt')
    assert(ospath.escape('file name.txt') == '"file name.txt"')
</pre>





### ospath.exists(path)

Returns *true* if `path` exists, *false* otherwise.

<pre>
    assert(ospath.exists('filename.txt') == true)
</pre>





### ospath.find_extension(path)

For the given `path`, return the index of the extension.  Returns `nil` if the `path` has no extension.

<pre>
    assert(ospath.find_extension('') == nil)
    assert(ospath.find_extension('filename') == nil)
    assert(ospath.find_extension('.lua') == 1)
    assert(ospath.find_extension('pathtests.lua') == 10)
    assert(ospath.find_extension('pathtests') == nil)
</pre>






### ospath.find_filename(path)

For the given `path`, return the index of the filename.

<pre>
    assert(ospath.find_filename('pathtests.lua') == 1)
    assert(ospath.find_filename('/pathtests') == 2)
    assert(ospath.find_filename('c:/pathtests') == 4)
    assert(ospath.find_filename('c:pathtests') == 3)
</pre>





### ospath.get_extension(path)

For the given `path`, return the extension.

<pre>
    assert(ospath.get_extension('') == '')
    assert(ospath.get_extension('filename') == '')
    assert(ospath.get_extension('filename.ext') == '.ext')
    assert(ospath.get_extension('filename.txt.ext') == '.ext')
</pre>





### ospath.get_filename(path)

For the given `path`, return just the filename without the directory.

<pre>
    assert(ospath.get_filename(''), '')
    assert(ospath.get_filename('filename'), 'filename')
    assert(ospath.get_filename('filename.ext'), 'filename.ext')
    assert(ospath.get_filename('c:/directory/filename.ext'), 'filename.ext')
    assert(ospath.get_filename('c:/directory/'), '')
</pre>





### ospath.is_directory(path)

Returns *true* if `path` is a directory, *false* otherwise.

<pre>
    assert(ospath.is_directory('') == false)
    assert(ospath.is_directory(scriptPath .. 'pathtests.lua') == false)
    assert(ospath.is_directory('.') == true)
    assert(ospath.is_directory('..') == true)
    assert(ospath.is_directory(scriptPath .. '../tests') == true)
    assert(ospath.is_directory(scriptPath .. '../tests/') == true)
</pre>





### ospath.is_file(path)

Returns *true* if `path` is a file, *false* otherwise.

<pre>
    assert(ospath.is_file('') == nil)
    assert(ospath.is_file(scriptPath .. 'pathtests.lua') == true)
    assert(ospath.is_file('.') == false)
    assert(ospath.is_file('..') == false)
    assert(ospath.is_file(scriptPath .. '../tests') == false)
    assert(ospath.is_file(scriptPath .. '../tests/') == nil)
</pre>





### ospath.is_relative(path)

Returns *true* if `path` is relative, *false* otherwise.

<pre>
    assert(ospath.is_relative('') == true)
    assert(ospath.is_relative('filename.ext') == true)
    assert(ospath.is_relative('/filename.ext') == false)
    assert(ospath.is_relative('c:/filename.ext') == false)
</pre>






### ospath.is_root(path)

Returns *true* if `path` is a root path, *false* otherwise.

<pre>
    assert(ospath.is_root('') == false)
    assert(ospath.is_root('filename.ext') == false)
    assert(ospath.is_root('/filename.ext') == true)
    assert(ospath.is_root('c:/filename.ext') == true)
</pre>






### ospath.is_unc(path)

Returns *true* if `path` is a UNC path, *false* otherwise.

<pre>
    assert(ospath.is_unc('') == false)
    assert(ospath.is_unc('filename.ext') == false)
    assert(ospath.is_unc('/filename.ext') == false)
    assert(ospath.is_unc('c:/filename.ext') == false)
    assert(ospath.is_unc('\\\\share') == true)
    assert(ospath.is_unc('//share') == true)
</pre>






### ospath.is_writable(path)

Returns *true* if `path` is writable, *false* if read-only.

<pre>
    local writable = ospath.is_writable('filename.dat')
</pre>





### ospath.make_absolute(path)

Converts `path` into an absolute path, simplifying the path as necessary.

Returns the new ospath.

<pre>
    local cwd = os.getcwd():gsub('\\', '/')
    assert(ospath.make_absolute('') == cwd)
    assert(ospath.make_absolute('.') == cwd .. '/')
    assert(ospath.make_absolute('..') == cwd:match('(.+)/') .. '/')
    assert(ospath.make_absolute('abc') == cwd .. '/abc')
</pre>






### ospath.make_backslash(path)

Convert any forward slashes in `path` to backslashes.

Returns the new ospath.

<pre>
    assert(ospath.make_backslash('') == '')
    assert(ospath.make_backslash(' ') == ' ')
    assert(ospath.make_backslash('\\\\abc') == '\\\\abc')
    assert(ospath.make_backslash('//abc') == '\\\\abc')
    assert(ospath.make_backslash('c:/abc/def/') == 'c:\\abc\\def\\')
</pre>






### ospath.make_slash(path)

Convert any backslashes in `path` to slashes.

Returns the new ospath.

<pre>
    assert(ospath.make_slash('') == '')
    assert(ospath.make_slash(' ') == ' ')
    assert(ospath.make_slash('\\\\abc') == '//abc')
    assert(ospath.make_slash('//abc') == '//abc')
    assert(ospath.make_slash('c:\\abc\\def\\') == 'c:/abc/def/')
</pre>






### ospath.make_writable(path)

Make the file *path* writable that is read-only.

Returns *true* if the process succeeded.

<pre>
    ospath.make_writable('filename.dat')
</pre>






### ospath.match(path, wildcard)

Returns *true* if the `wildcard` matches `path`, *false* otherwise.

<pre>
    assert(ospath.match('', '') == true)
    assert(ospath.match('', '*') == true)
    assert(ospath.match('', '*.*') == false)
    assert(ospath.match('', 'a*') == false)
    assert(ospath.match('abcdefg.txt', 'a*') == true)
    assert(ospath.match('abcdefg.txt', 'a*b*c?e*') == true)
    assert(ospath.match('abcdefg.txt', 'a*b*c?f*') == false)
    assert(ospath.match('abcdefg.txt', '*.') == false)
    assert(ospath.match('abcdefg.txt', '*.t') == false)
    assert(ospath.match('abcdefg.txt', '*.t*') == true)
    assert(ospath.match('abcdefg.txt', '*.t') == false)
    assert(ospath.match('abcdefg.txt', '*.*t') == true)
    assert(ospath.match('abcdefg.txt', '*.*x') == false)
    assert(ospath.match('abcdefg.txt', '*.txt') == true)
</pre>






### ospath.remove_directory(path)

Removes the directory component from `path`.

Returns the new ospath.

<pre>
    assert(ospath.remove_directory('') == '')
    assert(ospath.remove_directory(' \t') == ' \t')
    assert(ospath.remove_directory('abc') == 'abc')
    assert(ospath.remove_directory('/abc.') == 'abc.')
    assert(ospath.remove_directory('/dir/abc.') == 'abc.')
    assert(ospath.remove_directory('c:/abc.') == 'abc.')
    assert(ospath.remove_directory('c:/dir/abc') == 'abc')
</pre>






### ospath.remove_extension(path)

Removes the extension component from `path`.

Returns the new ospath.

<pre>
    assert(ospath.remove_extension('') == '')
    assert(ospath.remove_extension(' \t') == ' \t')
    assert(ospath.remove_extension('abc') == 'abc')
    assert(ospath.remove_extension('abc.') == 'abc')
    assert(ospath.remove_extension('abc.txt') == 'abc')
    assert(ospath.remove_extension('abc.txt.dat') == 'abc.txt')
</pre>






### ospath.remove_filename(path)

Removes the filename component from `path`.

Returns the new ospath.

<pre>
    assert(ospath.remove_filename('') == '')
    assert(ospath.remove_filename(' \t') == '')
    assert(ospath.remove_filename('abc') == '')
    assert(ospath.remove_filename('/abc.') == '/')
    assert(ospath.remove_filename('/dir/abc.') == '/dir/')
    assert(ospath.remove_filename('c:/abc.') == 'c:/')
    assert(ospath.remove_filename('c:/dir/abc') == 'c:/dir/')
</pre>






### ospath.remove_slash(path)

Removes the trailing slash from `path`.

Returns the new ospath.

<pre>
    assert(ospath.remove_slash('abc') == 'abc')
    assert(ospath.remove_slash('abc/') == 'abc')
</pre>






### ospath.replace_extension(path, extension)

Replaces the extension of `path` with the new one specified in `extension`.

Returns the new ospath.

<pre>
    assert(ospath.replace_extension('', 'ext') == '.ext')
    assert(ospath.replace_extension('', '.ext') == '.ext')
    assert(ospath.replace_extension('hello', 'ext') == 'hello.ext')
    assert(ospath.replace_extension('hello', '.ext') == 'hello.ext')
    assert(ospath.replace_extension('hello.txt', 'ext') == 'hello.ext')
    assert(ospath.replace_extension('hello.txt', '.ext') == 'hello.ext')
    assert(ospath.replace_extension('hello.txt.dat', 'ext') == 'hello.txt.ext')
    assert(ospath.replace_extension('hello.txt.dat', '.ext') == 'hello.txt.ext')
</pre>






### ospath.simplify(path)

Simplifies `path` by collapsing `.` and `..` directories and removing multiple directory slashes.

Returns the new ospath.

<pre>
    assert(ospath.simplify('') == '')
    assert(ospath.simplify('abc') == 'abc')
    assert(ospath.simplify('.abc') == '.abc')
    assert(ospath.simplify('./abc') == 'abc')
    assert(ospath.simplify('..abc') == '..abc')
    assert(ospath.simplify('../abc') == '../abc')
    assert(ospath.simplify('abc/////def') == 'abc/def')
    assert(ospath.simplify('abc/././././def') == 'abc/def')
    assert(ospath.simplify('c:/somedir/.././././def') == 'c:/def')
    assert(ospath.simplify('abc/.././././def') == 'def')
    assert(ospath.simplify('abc/ABC/../../../../def') == 'def')
    assert(ospath.simplify('c:\\abc\\ABC\\../..\\../..\\def') == 'c:/def')
    assert(ospath.simplify('\\\\uncserver\\pathA\\file.txt') == '\\\\uncserver/pathA/file.txt')
</pre>






### ospath.split(path)

Splits `path` into directory and filename components.  Returns both the directory and filename.

<pre>
    function compare_split(path, expectedDirname, expectedFilename)
    	local actualDirname, actualFilename = ospath.split(path)
    	return actualDirname == expectedDirname  and  actualFilename == expectedFilename
    end

    assert(compare_split('', '', '') == true)
    assert(compare_split('abc.txt', '', 'abc.txt') == true)
    assert(compare_split('/', '/', '') == true)
    assert(compare_split('/abc', '/', 'abc') == true)
    assert(compare_split('/abc/', '/abc/', '') == true)
    assert(compare_split('c:/', 'c:/', '') == true)
    assert(compare_split('c:/abc', 'c:/', 'abc') == true)
    assert(compare_split('c:/abc/', 'c:/abc/', '') == true)
</pre>






### ospath.trim(path)

Trims whitespace from the beginning and end of `path`.

Returns the new ospath.

<pre>
    assert(ospath.trim('abc.txt') == 'abc.txt')
    assert(ospath.trim(' abc.txt') == 'abc.txt')
    assert(ospath.trim('abc.txt ') == 'abc.txt')
    assert(ospath.trim('  \t  abc.txt \t \t \t ') == 'abc.txt')
</pre>






### ospath.unescape(path)

Unescapes `path`.

Returns the new ospath.

<pre>
    assert(ospath.unescape('') == '')
    assert(ospath.unescape('"') == '')
    assert(ospath.unescape('""') == '')
    assert(ospath.unescape('" "') == ' ')
    assert(ospath.unescape('"file with spaces') == 'file with spaces')
    assert(ospath.unescape('"file with spaces"') == 'file with spaces')
</pre>






