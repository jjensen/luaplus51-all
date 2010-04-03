-- Load the pipe module.
require 'pipe.core'

function pipe.execute(cmd, show, raw)
	if type(cmd) ~= 'string' then
		return nil
	end

	local output = {}
	output.lines = {}
	output.errlines = {}
	local p = pipe.popen(cmd, show)
	if not raw then
		while true do
			local line = p.stdout:read()
			if not line then
				break
			end
			output.lines[#output.lines + 1] = line
		end
	else
		output.lines[#output.lines + 1] = p.stdout:read('*a')
	end
	if not raw then
		while true do
			local line = p.stderr:read()
			if not line then
				break
			end
			output.errlines[#output.errlines + 1] = line
		end
	else
		output.errlines[#output.errlines + 1] = p.stderr:read('*a')
	end
	pipe.pclose(p)
	output.exitcode = p.exitcode
	return output
end

function pipe.lines(file)
	if type( file ) ~= 'file' then
		file = file or ""
		file = pipe.popen( file )
	end
	return function ()
		return file.stdout:read() or (assert(pipe.pclose(file)) and nil)
	end
end

function pipe.errlines(file)
	if type( file ) ~= 'file' then
		file = file or ""
		file = pipe.popen( file )
	end
	return function ()
		return file.stderr:read() or (assert(pipe.pclose(file)) and nil)
	end
end

function pipe.alllines(file)
	if type( file ) ~= 'file' then
		file = file or ""
		file = pipe.popen( file )
	end
	return function ()
		return file.stdout:read() or file.stderr:read() or (assert(pipe.pclose(file)) and nil)
	end
end
