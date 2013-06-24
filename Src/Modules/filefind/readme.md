## Overview

*filefind* is a Lua module providing both highly efficient directory iteration and an expressive syntax for recursively globbing sets of files or directories from the disk in a single execution.  The globbing functionality was inspired by Perforce's recursive wildcard syntax and implements JPSoft's TCC syntax for stepping up directories.

## Example

<pre>
    require 'filefind'

    for entry in filefind.glob('**') do
        print(entry.filename)
    end
</pre>


## Reference Manual

This is a reference of all of the filefind module's methods.

## Module `filefind`

**entryTable = filefind.attributes(*filename*)**

Retrieves file or directory properties for one item.


**for entry in filefind.glob(*pattern*) do**

Begins a new iteration of files and/or directories using *pattern* as the glob wildcard.  All glob syntax, described elsewhere, is available.


**for entry in filefind.match(*pattern*) do**

Begins a new iteration of files and directories using *pattern* as the wildcard.  Simple access to the file system is used.  The more powerful file globbing facilities are not available.


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


