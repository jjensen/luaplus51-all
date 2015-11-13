local M = require 'osprocess.core'

-- ex.parsecommandline
function M.parsecommandline(commandline)
	local lpeg = require 'lpeg'
	local field = lpeg.C('\"' * (lpeg.P(1) - '\"')^0 * '\"' + (1 - lpeg.P' ')^0)
	return lpeg.Ct(field * (lpeg.P(' ')^1 * field)^0 * -1):match(commandline)
end

-- ex.popen
function M.popen(args, binary)
	local out_rd, out_wr = M.pipe(not binary)
	args.stdout = out_wr
	if args.stderr_to_stdout then
		args.stderr = out_wr
	end
	local proc, err = M.spawn(args)
	out_wr:close()
	if not proc then
		out_rd:close()
		return proc, err
	end
	return proc, out_rd
end

-- ex.lines
function M.lines(args, binary)
	local proc, input = M.popen(args, not binary)
	if proc then
		return function()
			local line = input:read("*l")
			if line then return line end
			input:close()
			args.exitcode = proc:wait()
		end
	else
		return function()
		end
	end
end

-- ex.rawlines
function M.rawlines(args, binary)
	local proc, input = M.popen(args, not binary)
	return function()
		local line = input:read(100)
		if line then return line end
		input:close()
		args.exitcode = proc:wait()
	end
end

-- ex.popen2()
function M.popen2(args, binary)
	local in_rd, in_wr = M.pipe(not binary)
	local out_rd, out_wr = M.pipe(not binary)
	args.stdin = in_rd
	args.stdout = out_wr
	if args.stderr_to_stdout then
		args.stderr = out_wr
	end
	local proc, err = M.spawn(args)
	in_rd:close(); out_wr:close()
	if not proc then
		in_wr:close(); out_rd:close()
		return proc, err
	end
	return proc, out_rd, in_wr
end

-- ex.collectlines
function M.collectlines(args)
	local lines = {}
	for line in M.lines(args) do
		lines[#lines + 1] = line
	end
	args.lines = lines
	return lines
end

return M

