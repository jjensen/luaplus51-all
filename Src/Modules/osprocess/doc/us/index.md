# Introduction




# Credits

The primary contents of this document and source come from <http://lua-users.org/wiki/ExtensionProposal>.





# osprocess API

Note that all these functions return the standard (nil,"error message") on failure and that, unless otherwise specified, they return (true) on success.






## Environment

### osprocess.getenv(varname)

Returns the value of the process environment variable `varname` or `nil` if the variable is not defined.

<pre>
    print(osprocess.getenv("PATH"))
</pre>



### osprocess.setenv(varname, value)

Sets the environment variable `varname` to be `value`.  If `value` is `nil`, the environment variable is removed.  Note: The change occurs only within the currently running osprocess.

<pre>
	print(osprocess.getenv("foo"))          -- Prints: nil
    print(osprocess.setenv("foo", "bar"))
	print(osprocess.getenv("foo"))          -- Prints: bar
    print(osprocess.setenv("foo", nil))
	print(osprocess.getenv("foo"))          -- Prints: nil
</pre>




### env = osprocess.environ()

Returns a copy of the environment as a Lua table of key-value pairs.

<pre>
    env = osprocess.environ()
    print(env.HOME)
</pre>



-------------------------



### rd, wr = osprocess.pipe()

Create a pipe; 'rd' and 'wr' are Lua file objects.




----------------


## Process control

### osprocess.sleep(seconds)
### osprocess.sleep(interval, unit)

Suspends program execution for interval/unit seconds; 'unit' defaults to 1 and either argument can be floating point. The particular sub-second precision is implementation-defined.

<pre>
    osprocess.sleep(3.8) -- sleep for 3.8 seconds
    local microseconds = 1e6
    osprocess.sleep(3800000, microseconds) -- sleep for 3800000 µs
    local ticks = 100
    osprocess.sleep(380, ticks) -- sleep for 380 ticks
</pre>





### proc = osprocess.spawn(filename)
### proc = osprocess.spawn{filename, [args-opts]}
### proc = osprocess.spawn{command=filename, [args-opts]}

Creates a child osprocess.

`filename` names a program. If the (implementation-defined) pathname is not absolute, the program is found through an implementation-defined search method (the PATH environment variable on most systems).

If specified, [args-opts] is one or more of the following keys:

* `[1]..[n]=` - the command line arguments
* `args=` - an array of command line arguments
* `env=` - a table of environment variables
* `stdin=`, `stdout=`, `stderr=` - io.file objects for standard input, output, and error respectively
* `shell=` - Set to `true` if this is a shell application.  Set to `false` if this is a GUI application.  Defaults to `true`.
* `show=` - Set to `true` to show the application.  Set to `false` to hide it.  Defaults to `true`.
* `detach=` - Set to `true` to detach the application from the parent.  Defaults to `true`.
* `suspended=` - Set to `true` to start the spawned application as suspended, requiring a call to `os.resume(process)` to resume the application.  Defaults to `false`.
* `can_terminate=` - Set to `true` to start the application in such a way that it is part of the current process's process tree and can be terminated.  Defaults to `false`.

It is an error if both integer keys and an 'args' key are specified.

An implementation may provide special behavior if a zeroth argument (options.args[0] or options[0]) is provided.

The returned 'proc' userdatum has the method `wait()`.

<pre>
    -- run the echo command
    proc = osprocess.spawn"/bin/echo"
    proc = osprocess.spawn{"/bin/echo", "hello", "world"}
    proc = osprocess.spawn{command="/bin/echo", "hello", "world"}

    -- run the id command
    vars = { LANG="fr_FR" }
    proc = osprocess.spawn{"/bin/id", "-un", env=vars}
    proc = osprocess.spawn{command="/bin/id", "-un", env=vars)

    -- Useless use of cat
    local rd, wr = assert(osprocess.pipe())
    local proc = assert(osprocess.spawn("/bin/cat", {stdin=rd}))
    rd:close()
    wr:write("Hello world\n")
    wr:close()
    proc:wait()

    -- Run a program with a modified environment
    local env = osprocess.environ()
    env.LUA_PATH = "/usr/share/lib/lua/?.lua"
    env.LUA_CPATH = "/usr/share/lib/lua/?.so"
    local proc = assert(osprocess.spawn("lua", {args = {"-e", 'print"Hello world\n"'}, env=env }))
    proc:wait()

    -- popen2()
    function popen2(...)
        local in_rd, in_wr = osprocess.pipe()
        local out_rd, out_wr = osprocess.pipe()
        local proc, err = osprocess.spawn{stdin = in_rd, stdout = out_wr, ...}
        in_rd:close(); out_wr:close()
        if not proc then
            in_wr:close(); out_rd:close()
            return proc, err
        end
        return proc, out_rd, in_wr
    end

    -- usage:
    local p, i, o = assert(popen2("wc", "-w"))
    o:write("Hello world"); o:close()
    print(i:read"*l"); i:close()
    p:wait()
</pre>



### osprocess.terminate(process)

Terminates a spawned process, if possible.

If `process` is a number or light userdata, `process` is expected to represent the Windows job the process tree was created under.

Otherwise, `process` should be a table with the entries from `proc:getinfo()`.




### info = proc:getinfo()

Returns information about the spawned osprocess.

On Windows, a table structure is returned with the following members:

* `process_handle` - Light userdata representing the Windows process HANDLE.
* `thread_handle` - Light userdata representing the Windows thread HANDLE.
* `process_id` - An integer representing the Windows process ID.
* `thread_id` - An integer representing the Windows thread ID.
* `job` - Light userdata representing the Windows job HANDLE if the process was created with `can_terminate = true`.



### proc:resume()

Resumes a suspended osprocess.



### exitcode = proc:wait()

Wait for child process termination; 'exitcode' is the code returned by the child osprocess.




### osprocess.collectlines{filename, [args-opts]}
### osprocess.collectlines{command=filename, [args-opts]}

Returns a table of all output from the new process's stdout.

See `osprocess.spawn()` for additional information.





### osprocess.lines{filename, [args-opts]}
### osprocess.lines{command=filename, [args-opts]}

Returns an iterator that reads the new process's stdout line by line.

See `osprocess.spawn()` for additional information.





### proc, input = osprocess.popen{filename, [args-opts]}
### proc, input = osprocess.popen{command=filename, [args-opts]}

Creates a child process with the process's stdout redirected to be read from `input`.

See `osprocess.spawn()` for additional information.





### proc, input, output = osprocess.popen2{filename, [args-opts]}
### proc, input, output = osprocess.popen2{command=filename, [args-opts]}

Creates a child process with the process's stdout redirected to be read from `input` and the process's stdin redirected to be written to through `output`.

See `osprocess.spawn()` for additional information.





### args = osprocess.parsecommandline(commandline)

Parses a single string command line into a table of arguments suitable for passing to `osprocess.spawn`.

<pre>
    args = osprocess.parsecommandline('dir "c:\\Program Files\\"')
</pre>


