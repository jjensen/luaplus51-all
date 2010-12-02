module(..., package.seeall)

require 'ex.core'

-- ex.parsecommandline
function ex.parsecommandline(commandline)
	require 'lpeg'
	local field = lpeg.C('\"' * (lpeg.P(1) - '\"')^0 * '\"' + (1 - lpeg.P' ')^0)
	return lpeg.Ct(field * (lpeg.P(' ')^1 * field)^0 * -1):match(commandline)
end

-- ex.popen
function ex.popen(args)
	local out_rd, out_wr = io.pipe()
	args.stdout = out_wr
	local proc, err = os.spawn(args)
	out_wr:close()
	if not proc then
		out_rd:close()
		return proc, err
	end
	return proc, out_rd
end

-- ex.lines
function ex.lines(args)
	local proc, input = popen(args)
	return function()
		local line = input:read("*l")
		if line then return line end
		input:close()
		proc:wait()
	end
end

-- ex.popen2()
function ex.popen2(args)
	local in_rd, in_wr = io.pipe()
	local out_rd, out_wr = io.pipe()
	args.stdin = in_rd
	args.stdout = out_wr
	local proc, err = os.spawn(args)
	in_rd:close(); out_wr:close()
	if not proc then
		in_wr:close(); out_rd:close()
		return proc, err
	end
	return proc, out_rd, in_wr
end

-- ex.collectlines
function ex.collectlines(args)
	local lines = {}
	for line in ex.lines(args) do
		lines[#lines + 1] = line
	end
	return lines
end


local function copy_directory_helper(srcPath, destPath, callback, deleteExtra)
	-- Create the destination directory.
	os.mkdir(destPath)

	-- Grab the destination directory contents.
	local destDirs = {}
	local destFiles = {}
	for destHandle in filefind.match(destPath .. "*.*") do
		local fileName = destHandle:GetFileName()
		if destHandle:IsDirectory() then
			if fileName ~= '.'  and  fileName ~= '..' then
				destDirs[fileName:lower()] = true
			end
		else
			destFiles[fileName:lower()] = destHandle:GetLastWriteTime()
		end
	end

	-- Scan the source directory.
	local srcDirs = {}
	local srcFiles = {}
	for srcHandle in filefind.match(srcPath .. "*.*") do
		local fileName = srcHandle:GetFileName()
		if srcHandle:IsDirectory() then
			if fileName ~= '.'  and  fileName ~= '..' then
				srcDirs[#srcDirs + 1] = fileName
				destDirs[fileName:lower()] = nil
			end
		else
			local lowerFileName = fileName:lower()
			if srcHandle:GetLastWriteTime() ~= destFiles[lowerFileName] then
				srcFiles[#srcFiles + 1] = fileName
			end
			destFiles[lowerFileName] = nil
		end
	end

	-- Delete any leftover destination files.
	if deleteExtra then
		for fileName in pairs(destFiles) do
			local destFullPath = destPath .. fileName
			print('del ' .. destFullPath)
			os.remove(destFullPath)
		end
	end

	-- Copy any changed files.
	for _, fileName in ipairs(srcFiles) do
		local srcFileName = srcPath .. fileName
		local destFileName = destPath .. fileName
		if callback then callback('copy', srcFileName, destFileName) end
		os.copyfile(srcFileName, destFileName)
	end

	-- Delete any leftover destination directories.
	if deleteExtra then
		for dirName in pairs(destDirs) do
			local destFullPath = destPath .. dirName .. '/'
			if callback then callback('del', destFullPath) end
			os.remove(destFullPath)
		end
	end

	-- Recurse through the directories.
	for _, dirName in ipairs(srcDirs) do
		copy_directory_helper(srcPath .. dirName .. '/', destPath .. dirName .. '/', callback, deleteExtra)
	end
end


function ex.copydirectory(srcPath, destPath, callback)
	require 'filefind'
	
	srcPath = os.path.add_slash(os.path.make_slash(srcPath))
	destPath = os.path.add_slash(os.path.make_slash(destPath))

	copy_directory_helper(srcPath, destPath, callback, false)
end


function ex.mirrordirectory(srcPath, destPath, callback)
	require 'filefind'

	srcPath = os.path.add_slash(os.path.make_slash(srcPath))
	destPath = os.path.add_slash(os.path.make_slash(destPath))

	copy_directory_helper(srcPath, destPath, callback, true)
end

