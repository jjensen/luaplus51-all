## Overview

*filefind* is a Lua module providing both highly efficient directory iteration and an expressive syntax for recursively globbing sets of files or directories from the disk in a single execution.  The globbing functionality was inspired by Perforce's recursive wildcard syntax and implements JPSoft's TCC syntax for stepping up directories.

## Example

<pre>
    filefind = require 'filefind'

    for entry in filefind.glob('**') do
        print(entry.filename)
    end
</pre>


## Reference Manual

This is a reference of all of the *filefind* module's methods.

## Module `filefind`

**entryTable = filefind.attributes(*filename*)**

Retrieves file or directory properties for one item.


**for entry in filefind.glob(*pattern*) do**

Begins a new iteration of files and/or directories using *pattern* as the glob wildcard.  All glob syntax, described elsewhere, is available.


**for entry in filefind.match(*pattern*) do**

Begins a new iteration of files and directories using *pattern* as the wildcard.  Simple access to the file system is used.  The more powerful file globbing facilities are not available.


**matches = filefind.pattern_match(*pattern*, *string*, *caseSensitive* = false, *recursive* = true) do**

Tests `string` against `pattern` to determine a match.  The syntax for `pattern` follows the wildcard globbing syntax described elsewhere in this document.

Pass `true` for `caseSensitive` to perform a search where case sensitivity matters.  The default is `false`.

Pass `true` for `recursive` to perform a search where directory separators are tested and `**` will work across directory separator boundaries.  Pass `false` to use `*` to search across the entire `string`.



### Time Conversion

**lowTime, highTime = filefind.unix\_time\_to\_FILETIME\_UTC(unixTime)**

Converts a Unix time_t to a Windows FILETIME adjusted to UTC.  Returns the FILETIME as a two integers representing low time and high time.


**entryTable = filefind.unix\_time\_to\_FILETIME\_UTC(unixTime)**

Converts a Unix time_t to a Windows FILETIME adjusted to UTC.  Returns the FILETIME as a two integers representing low time and high time.


**entryTable = filefind.FILETIME\_to\_unix\_time\_UTC(filetime)**

Converts a Windows FILETIME to Unix time_t adjusted to UTC.  `filetime` may be two integers representing the low time and high time of the FILETIME structure or a string representing the low time and high time combined.

Returns the time_t.


**entryTable = filefind.time\_t\_to\_FILETIME(time_t)**

Converts a Unix time_t to a Windows FILETIME.  Returns the FILETIME as a two integers representing low time and high time.


**entryTable = filefind.FILETIME\_to\_time\_t(filetime)**

Converts a Windows FILETIME to Unix time_t.  `filetime` may be two integers representing the low time and high time of the FILETIME structure or a string representing the low time and high time combined.

Returns the time_t.




### File entry information

**entry.table** - table - Returns a table with all of the properties below.  Entry properties are usually looked up and returned on demand.  Some may take extra time to compute.

**entry.filename** - string - The relative path from the *filefind.match()* starting directory.

**entry.creation_time** - number - The creation time of the file/directory in seconds starting from midnight (00:00:00), January 1, 1970.

**entry.access_time** - number - The last access time of the file/directory in seconds starting from midnight (00:00:00), January 1, 1970.

**entry.write_time** - number - The last write time of the file/directory in seconds starting from midnight (00:00:00), January 1, 1970.

**entry.creation_FILETIME** - table - The creation time of the file/directory in FILETIME format starting from midnight (00:00:00), January 1, 1970.  Element 1 of the table is the low file time.  Element 2 is the high file time.

**entry.access_FILETIME** - table - The last access time of the file/directory in FILETIME format starting from midnight (00:00:00), January 1, 1970.  Element 1 of the table is the low file time.  Element 2 is the high file time.

**entry.write_FILETIME** - table - The last write time of the file/directory in FILETIME format starting from midnight (00:00:00), January 1, 1970.  Element 1 of the table is the low file time.  Element 2 is the high file time.

**entry.is_directory** - boolean - <tt>true</tt> if the glob entry is a directory, <tt>false</tt> otherwise.

**entry.is_link** - boolean - <tt>true</tt> if the entry is a hard or symbolic link, <tt>false</tt> otherwise.

**entry.is_readonly** - boolean - <tt>true</tt> if the glob entry is read only, <tt>false</tt> otherwise.

**entry.number_of_links** - number - The number of files pointing to the hardlink blob, 0 if none.

**entry.size** - number - The size of the file.



## Available Glob Syntax

<table border="1" width="75%" align="center">
  <tr>

    <td width="150"><b>Wildcard</b></td>
    <td width="450"><b>Description</b></td>
  </tr>
  <tr>
    <td><code>?</code></td>
    <td>Matches any single character of the file name or directory name.</td>
  </tr>

  <tr>
    <td><code>*</code></td>
    <td>Matches 0 or more characters of the file name or directory name.</td>
  </tr>
  <tr>
    <td><code>/</code> at end of pattern</td>
    <td>Any pattern with a closing slash will start a directory search, instead of the default file search.</td>
  </tr>
  <tr>
    <td><code>**</code></td>
    <td>Search files recursively.</td>
  </tr>
  <tr>
    <td><code>**/</code></td>
    <td>Search directories recursively.</td>
  </tr>
</table>

<p>Some examples follow:</p>

<table border="1" width="75%"  align="center">
  <tr>
    <td width="150"><b>Example Pattern</b></td>
    <td width="450"><b>Description</b></td>
  </tr>

  <tr>
    <td><code>File.txt</code></td>
    <td>Matches a file or directory called <i>File.txt</i>.</td>
  </tr>
  <tr>
    <td><code>File*.txt</code></td>

    <td>Matches any file or directory starting with File and ending with a .txt extension.</td>
  </tr>
  <tr>
    <td><code>File?.txt</code></td>
    <td>Matches any file or directory starting with File and containing one more character.</td>
  </tr>
  <tr>
    <td><code>F??e*.txt</code></td>
    <td>Matches a file or directory starting with F, followed by any two characters, followed by e, then any number of characters up to the extension .txt.</td>
  </tr>
  <tr>
    <td><code>File*</code></td>
    <td>Matches a file or directory starting with File and ending with or without an extension.</td>
  </tr>

  <tr>
    <td><code>*</code></td>
    <td>Matches all files (non-recursive).</td>
  </tr>
  <tr>
    <td><code>*/</code></td>
    <td>Matches all directories (non-recursive).</td>

  </tr>
  <tr>
    <td><code>A*/</code></td>
    <td>Matches any directory starting with A (non-recursive).</td>
  </tr>
  <tr>
    <td><code>**/*</code></td>

    <td>Matches all files (recursive).</td>
  </tr>
  <tr>
    <td><code>**</code></td>
    <td>Shortened form of above. Matches all files (recursive). Internally, expands to **/*</td>
  </tr>
  <tr>

    <td><code>**/</code></td>
    <td>Matches all directories (recursive).</td>
  </tr>
  <tr>
    <td><code>**{filename chars}</code></td>
    <td>Matches {filename chars} recursively. Internally, expands to **/*{filename chars}.</td>
  </tr>

  <tr>
    <td><code>{dirname chars}**</code></td>
    <td>Expands to {dirname chars}*/**.</td>
  </tr>
  <tr>
    <td><code>{dirname chars}**{filename chars}</code></td>
    <td>Expands to {dirname chars}*/**/*{filename chars}.</td>

  </tr>
  <tr>
    <td><code>**.h</code></td>
    <td>Matches all *.h files recursively. Expands to **/*.h.</td>
  </tr>
  <tr>
    <td><code>**resource.h</code></td>

    <td>Matches all *resource.h files recursively.
    Expands to **/*resource.h.</td>
  </tr>
  <tr>
    <td><code>BK**</code></td>
    <td>Matches all files in any directory starting with BK, recursively. Expands to BK*/**.</td>
  </tr>
  <tr>

    <td><code>BK**.h</code></td>
    <td>Matches all *.h files in any directory starting with BK, recursively. Expands to BK*/**/*.h.</td>
  </tr>
  <tr>
    <td><code>c:/Src/**/*.h</code></td>
    <td>Matches all *.h files recursively, starting at c:/Src/.</td>
  </tr>
  <tr>
    <td><code>c:/Src/**/*Grid/</code></td>
    <td>Recursively matches all directories under c:/Src/ that end with Grid.</td>
  </tr>
  <tr>
    <td><code>c:/Src/**/*Grid*/</code></td>
    <td>Recursively matches all directories under c:/Src/ that contain Grid.</td>
  </tr>
  <tr>
    <td><code>c:/Src/**/*Grid*/**/ABC/**/Readme.txt</code></td>

    <td>Recursively matches all directories under c:/Src/ that contain Grid. From the found directory, recursively matches directories until ABC/ is found. From there, the file <i>Readme.txt</i> is searched for recursively. </td>
  </tr>
</table>

<p>Finally, a couple flags are available. Flags are appended at the end of the pattern line. Each flag begins with an @ character. Spaces should not be inserted between flags unless they are intended as part of the string literal.</p>

<table border="1" width="75%" align="center">
  <tr>
    <td width="150"><b>Flags and Other Expansions</b></td>
    <td width="450"><b>Description</b></td>
  </tr>
  <tr>
    <td><code>@*</code></td>
    <td>Search files and directories recursively.</td>
  </tr>
  <tr>
    <td><code>@-pattern</code></td>
    <td>Adds <code>pattern</code> to the file ignore list. Any file matching a pattern in the file ignore list is removed from the search.</td>
  </tr>
  <tr>
    <td><code>@-pattern/</code></td>
    <td>Adds <code>pattern/</code> to the directory ignore list. Any directory matching a pattern in the directory ignore list is removed from the search.</td>
  </tr>
  <tr>
    <td><code>@=pattern</code></td>
    <td>Adds <code>pattern</code> to the exclusive file list. Any file not matching a pattern in the exclusive file list is automatically removed from the search.</td>
  </tr>
  <tr>
    <td><code>@=pattern/</code></td>
    <td>Adds <code>pattern/</code> to the exclusive directory list. Any directory not matching a pattern in the exclusive file list is automatically removed from the search.</td>
  </tr>
  <tr>
    <td>More than two periods for going up parent directories.</td>
    <td>Similar to 4DOS, each period exceeding two periods goes up one additional parent directory. So, a 4 period path expands to <code>../../../.</code></td>
  </tr>
</table>

Wildcards may appear anywhere in the pattern, including directories.

    */*/*/*.c

Note that *.* only matches files that have an extension.  This is different than standard DOS behavior.  Use * all by itself to match files, extension or not.

Recursive wildcards can be used anywhere:

    c:/Dir1/**/A*/**/FileDirs*/**.mp3

This matches all directories under c:/Dir1/ that start with A.  Under all of the directories that start with A, directories starting with FileDirs are matched recursively.  Finally, all files ending with an mp3 extension are matched.

<p>A few examples:</p>

<table border="1" width="75%" align="center">
  <tr>
    <td width="150"><b>Example Pattern</b></td>
    <td width="450"><b>Description</b></td>
  </tr>
  <tr>
    <td><code>Src/**/@-**/.git/@-**/.svn/</code></td>
    <td>Recursively lists all directories under Src/, but directories called .git/ and .svn/ are filtered.</td>
  </tr>
  <tr>
    <td><code>Src/**@=**.lua@=**/README</code></td>
    <td>Recursively lists all files under Src/ which match *.lua or README. All other files are ignored.</td>
  </tr>
  <tr>
    <td><code>Src/**/@-**/.git/@-**/.svn/@=**.lua@=**/README</code></td>
    <td>Recursively lists all files under Src/ which match *.lua or README. The versions of those files that may exist in .git/ or .svn/ are ignored.</td>
  </tr>
  <tr>
    <td><code>...../StartSearchHere/**</code></td>
    <td>Expands to: ../../../../StartSearchHere/**</td>
  </tr>
</table>



## License

The filefind module is licensed under the terms of the MIT and BSD licenses.


## Credits

The filefind module glob source code was originally based on [Matthias Wandel's MyGlob](http://www.sentex.net/~mwandel/jhead/) code, used in his Exif Jpeg camera setting parser and thumbnail remover application.  It also expands upon the [wildcmp()](http://www.codeproject.com/string/wildcmp.asp) code written by Jack Handy.

For version 3.0, the MyGlob implementation was replaced with Ruby's faster implementation found in dir.c of the Ruby source code distribution.

Joshua Jensen wrote the rest of the filefind module.


## History

* Version 3.0 (23 Jan 2014)
    * Updated to Ruby's dir.c glob() implementation.
    * The @- and @= modifiers use a recursive pattern matching syntax.

* Version 2.0 (9 Dec 2009)
    * Adds a revamped version of the old 'glob' module that handles iterators and has even better performance.  Also adds support for the <tt>+</tt> modifier that will list both files and directories.
    * Changes the access functions to just member lookups for the resultant handle.
    * A number of bug fixes.

* Version 1.0 (27 Aug 2002 - CodeProject article at http://www.codeproject.com/KB/files/fileglob.aspx)
    * Initial version.


