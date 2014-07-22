local M = {}

local filefind

local function copy_directory_helper(srcPath, destPath, options)
    local callback = options.callback
    local noop = options.noop
    if not noop then
        -- Create the destination directory.
        os.mkdir(destPath)
    end

    -- Grab the destination directory contents.
    local destDirs = {}
    local destFiles = {}
    for destHandle in filefind.match(destPath .. "*.*") do
        local fileName = destHandle.filename
        if destHandle.is_directory then
            if fileName ~= '.'  and  fileName ~= '..' then
                destDirs[fileName:lower()] = true
            end
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
            if fileName ~= '.'  and  fileName ~= '..' then
                srcDirs[#srcDirs + 1] = fileName
                destDirs[fileName:lower()] = nil
            end
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
    for _, fileName in ipairs(srcFiles) do
        local srcFileName = srcPath .. fileName
        local destFileName = destPath .. fileName
        if callback then callback('copy', srcFileName, destFileName) end
        if not noop then
            os.chmod(destFileName, 'w')                -- Make sure we can overwrite the file
            os.copyfile(srcFileName, destFileName)
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


function M.copy_directory(srcPath, destPath, options)
    if not filefind then
        filefind = require 'filefind'
    end

    srcPath = os.path.add_slash(os.path.make_slash(srcPath))
    destPath = os.path.add_slash(os.path.make_slash(destPath))

    copy_directory_helper(srcPath, destPath, options)
end


function M.mirror_directory(srcPath, destPath, callback)
    if not filefind then
        filefind = require 'filefind'
    end

    srcPath = os.path.add_slash(os.path.make_slash(srcPath))
    destPath = os.path.add_slash(os.path.make_slash(destPath))

    copy_directory_helper(srcPath, destPath, callback, { deleteExtra = true })
end


function M.remove_empty_directories(path)
    if not filefind then
        filefind = require 'filefind'
    end

    local dirs = {}
    local remove = true

    for handle in filefind.match(path .. "*.*") do
        if handle.is_directory then
            local fileName = handle.filename

            if fileName ~= '.'  and  fileName ~= '..' then
                dirs[#dirs + 1] = fileName
            end
        else
            remove = false
        end
    end

    for _, dirName in ipairs(dirs) do
        if not ex.removeemptydirectories(path .. dirName .. '\\') then
            remove = false
        end
    end

    if remove then
        os.remove(path)
    end

    return remove
end

function M.read_file(filename)
    local file, err = io.open(filename, 'rb')
    if not file then return nil, err end
    local buffer = file:read('*a')
    file:close()
    return buffer
end

function M.write_file(filename, buffer)
    local file, err = io.open(filename, 'wb')
    if not file then return nil, err end
    local result, err = file:write(buffer)
    if not result then return result, err end
    file:close()
end

local core = require 'ex.path.core'
for key, value in pairs(core) do
    M[key] = value
end

return M

