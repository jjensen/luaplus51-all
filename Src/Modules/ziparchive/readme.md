## Introduction

The module *ziparchive* allows high performance read and write access to .zip files.



## Example

<pre>
    require 'ziparchive'

    local archive = ziparchive.open('myfile.zip')
    for entry in archive.files() do
    	print(entry)
    end
</pre>



