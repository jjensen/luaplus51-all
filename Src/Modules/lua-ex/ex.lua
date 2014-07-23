local M = require 'ex.core'

-- ex.parsecommandline
function M.parsecommandline(commandline)
	require 'lpeg'
	local field = lpeg.C('\"' * (lpeg.P(1) - '\"')^0 * '\"' + (1 - lpeg.P' ')^0)
	return lpeg.Ct(field * (lpeg.P(' ')^1 * field)^0 * -1):match(commandline)
end

-- ex.popen
function M.popen(args, binary)
	local out_rd, out_wr = io.pipe(not binary)
	args.stdout = out_wr
	if args.stderr_to_stdout then
		args.stderr = out_wr
	end
	local proc, err = os.spawn(args)
	out_wr:close()
	if not proc then
		out_rd:close()
		return proc, err
	end
	return proc, out_rd
end

-- ex.lines
function M.lines(args, binary)
	local proc, input = popen(args, not binary)
	return function()
		local line = input:read("*l")
		if line then return line end
		input:close()
		args.exitcode = proc:wait()
	end
end

-- ex.rawlines
function M.rawlines(args, binary)
	local proc, input = popen(args, not binary)
	return function()
		local line = input:read(100)
		if line then return line end
		input:close()
		args.exitcode = proc:wait()
	end
end

-- ex.popen2()
function M.popen2(args, binary)
	local in_rd, in_wr = io.pipe(not binary)
	local out_rd, out_wr = io.pipe(not binary)
	args.stdin = in_rd
	args.stdout = out_wr
	if args.stderr_to_stdout then
		args.stderr = out_wr
	end
	local proc, err = os.spawn(args)
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
	for line in ex.lines(args) do
		lines[#lines + 1] = line
	end
	args.lines = lines
	return lines
end

local filefind

local function copy_directory_helper(srcPath, destPath, options)
	local callback = options.callback
	local noop = options.noop
	-- Grab the destination directory contents.
	local destDirs = {}
	local destFiles = {}
	for destHandle in filefind.match(destPath .. "*.*") do
		local fileName = destHandle.filename
		if destHandle.is_directory then
			destDirs[fileName:lower()] = true
		else
			destFiles[fileName:lower()] = destHandle.write_time
		end
	end

	-- Scan the source directory.
	local check_timestamp = options.check_timestamp
	local srcDirs = {}
	local srcFiles = {}
	for srcHandle in filefind.match(srcPath .. "*.*") do
		local fileName = srcHandle.filename
		if srcHandle.is_directory then
			srcDirs[#srcDirs + 1] = fileName
			destDirs[fileName:lower()] = nil
		else
			local lowerFileName = fileName:lower()
			if check_timestamp then
				if srcHandle.write_time ~= destFiles[lowerFileName] then
					srcFiles[#srcFiles + 1] = fileName
				end
			else
				if not destFiles[lowerFileName] then
					srcFiles[#srcFiles + 1] = fileName
				end
			end
			destFiles[lowerFileName] = nil
		end
	end

	-- Delete any leftover destination files.
	if options.deleteExtra then
		table.sort(destFiles)
		for fileName in pairs(destFiles) do
			local destFullPath = destPath .. fileName
			if callback then callback('del', destFullPath) end
			if not noop then
				os.remove(destFullPath)
			end
		end
	end

	-- Copy any changed files.
	table.sort(srcFiles)
	local copyfile
	if not noop  and  srcFiles[1] then
		-- Create the destination directory.
		if callback then callback('mkdir', destPath) end
		os.mkdir(destPath)
		copyfile = options.copyfile  or  os.copyfile
	end

	for _, fileName in ipairs(srcFiles) do
		local srcFileName = srcPath .. fileName
		local destFileName = destPath .. fileName
		if callback then callback('copy', srcFileName, destFileName) end
		if not noop then
			os.chmod(destFileName, 'w')				-- Make sure we can overwrite the file
			os.remove(destFileName)                 -- Break any hardlinks/symlinks...
			copyfile(srcFileName, destFileName)
		end
	end

	-- Delete any leftover destination directories.
	if options.deleteExtra then
		table.sort(destDirs)
		for dirName in pairs(destDirs) do
			local destFullPath = destPath .. dirName .. '/'
			if callback then callback('del', destFullPath) end
			if not noop then
				os.remove(destFullPath)
			end
		end
	end

	-- Recurse through the directories.
	table.sort(srcDirs)
	for _, dirName in ipairs(srcDirs) do
		copy_directory_helper(srcPath .. dirName .. '/', destPath .. dirName .. '/', options)
	end
end


function M.copydirectory(srcPath, destPath, options)
	if not filefind then
		filefind = require 'filefind'
	end

	srcPath = os.path.add_slash(os.path.make_slash(srcPath))
	destPath = os.path.add_slash(os.path.make_slash(destPath))

	if options.hardlink then
		options.copyfile = function(src, dest)
			return os.hardlink(dest, src)
		end
	end
	copy_directory_helper(srcPath, destPath, options)
end


function M.mirrordirectory(srcPath, destPath, options)
	if not filefind then
		filefind = require 'filefind'
	end

	srcPath = os.path.add_slash(os.path.make_slash(srcPath))
	destPath = os.path.add_slash(os.path.make_slash(destPath))

	options.deleteExtra = true
	if options.hardlink then
		options.copyfile = function(src, dest)
			return os.hardlink(dest, src)
		end
	end
	copy_directory_helper(srcPath, destPath, options)
end


function M.removeemptydirectories(path)
	if not filefind then
		filefind = require 'filefind'
	end

	local dirs = {}
	local remove = true

	for handle in filefind.match(os.path.combine(path, "*")) do
		if handle.is_directory then
			dirs[#dirs + 1] = handle.filename
		else
			remove = false
		end
	end

	for _, dirName in ipairs(dirs) do
		if not ex.removeemptydirectories(os.path.combine(path, dirName)) then
			remove = false
		end
	end

	if remove then
		os.remove(path)
	end

	return remove
end

function M.readall(filename)
	local file, err = io.open(filename, 'rb')
	if not file then return nil, err end
	local buffer = file:read('*a')
	file:close()
	return buffer
end

function M.writeall(filename, buffer)
	local file, err = io.open(filename, 'wb')
	if not file then return nil, err end
	local result, err = file:write(buffer)
	if not result then return result, err end
	file:close()	
end

return M

