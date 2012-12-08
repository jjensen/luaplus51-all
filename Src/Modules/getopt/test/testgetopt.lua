getopt = require 'getopt'

do
	local options = getopt.makeOptions{
		getopt.Option {{"gen"}, "Set a project generator", "Req", 'GENERATOR'},
		getopt.Option {{"gui"}, "Pop up a GUI to set options"},
		getopt.Option {{"compiler"}, "Set the default compiler used to build with", "Req", 'COMPILER'},
		getopt.Option {{"postfix"}, "Extra text for the IDE project name"},
		getopt.Option {{"config"}, "Filename of additional configuration file", "Req", 'CONFIG'},
		getopt.Option {{"jamflags"}, "Extra flags to make available for each invocation of Jam.  Specify in KEY=VALUE form.", "Req", 'JAMBASE_FLAGS' },
		getopt.Option {{"jamexepath"}, "The full path to the Jam executable when the default location won't suffice.", "Req", 'JAMEXEPATH' },
	}

	local nonOpts, opts, errors = getopt.getOpt (arg, options)
	if #errors > 0  or
		(#nonOpts ~= 1  and  #nonOpts ~= 2)
	then
		print (table.concat (errors, "\n") .. "\n" ..
				getopt.usageInfo ("Usage: jam --workspace [options] <source-jamfile> <path-to-destination>",
				options))
		os.exit(-1)
	end
end

if false then
  options = getopt.makeOptions ({
                           getopt.Option {{"verbose", "v"}, "verbosely list files"},
                           getopt.Option {{"output", "o"}, "dump to FILE", "Opt", "FILE"},
                           getopt.Option {{"name", "n"}, "only dump USER's files", "Req", "USER"},
                       })

  function test (cmdLine)
    local nonOpts, opts, errors = getopt.getOpt (cmdLine, options)
    if #errors == 0 then
      print ("options=" .. tostring (opts) ..
             "  args=" .. tostring (nonOpts) .. "\n")
    else
      print (table.concat (errors, "\n") .. "\n" ..
             getopt.usageInfo ("Usage: foobar [OPTION...] FILE...",
                               options))
    end
  end

  -- FIXME: Turn the following documentation into unit tests
  prog = {name = "foobar"} -- for errors
  -- Example runs:
  test {"foo", "-v"}
  -- options={verbose={1}}  args={1=foo}
  test {"foo", "--", "-v"}
  -- options={}  args={1=foo,2=-v}
  test {"-o", "-V", "-name", "bar", "--name=baz"}
  -- options={name={"baz"},version={1},output={1}}  args={}
  test {"-foo"}
  -- unrecognized option `-foo'
  -- Usage: foobar [OPTION]... [FILE]...
  --
  --   -v, -verbose                verbosely list files
  --   -o, -output[=FILE]          dump to FILE
  --   -n, -name=USER              only dump USER's files
  --   -V, -version                output version information and exit
  --   -h, -help                   display this help and exit


  getopt.usage()
end

