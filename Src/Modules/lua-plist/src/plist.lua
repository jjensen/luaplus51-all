local M = {}

local plistDictMetatable = {
    __index = function(self, key)
        local lowerKey = key:lower()
        for index = 1, #self do
            local entry = rawget(self, index)
            if entry.key:lower() == lowerKey then
                return entry.value
            end
        end
    end,

    __newindex = function(self, key, value)
        local lowerKey = key:lower()
        for index = 1, #self do
            local entry = rawget(self, index)
            if entry.key:lower() == lowerKey then
                entry.value = value
                return
            end
        end
        rawset(self, #self + 1, { key = key, value = value })
    end
}

local function RecurseEntry(entry)
    local isDictionary = false
    local output = {}
    local key
    local value
    for _, subEntry in ipairs(entry) do
        if type(subEntry) == 'table' then
            if subEntry.tag == 'key' then
                key = subEntry[1]
            elseif subEntry.tag == 'string' then
                value = subEntry[1] or ''
            elseif subEntry.tag == 'false' then
                value = false
            elseif subEntry.tag == 'true' then
                value = true
            elseif subEntry.tag == 'integer' then
                value = tonumber(subEntry[1])
            elseif subEntry.tag == 'array' then
                value = RecurseEntry(subEntry)
            elseif subEntry.tag == 'dict' then
                value = RecurseEntry(subEntry)
            end
        end

        if value ~= nil then
            if key then
                output[#output + 1] = { key = key, value = value }
                isDictionary = true
            else
                output[#output + 1] = value
            end

            key = nil
            value = nil
        end
    end

    if isDictionary then
        setmetatable(output, plistDictMetatable)
    end

    return output
end

function M.read(filename)
    local lom = require 'lxp.lom'
    local file = io.open(filename, 'rb')
    local buffer = file:read('*a')
    file:close()
    local info, err = lom.parse(buffer:match('(<%?xml .+</plist>)'))

    if info.tag == 'plist' then
        for _, entry in ipairs(info) do
            if type(entry) == 'table' then
                return setmetatable(RecurseEntry(entry), plistDictMetatable)
            end
        end
    end

    return setmetatable({}, plistDictMetatable)
end


local function ExportDict(entry, out, spaces)
    local origSpaces = spaces
    out[#out + 1] = origSpaces .. '<dict>\n'
    spaces = spaces .. '  '
    for index = 1, #entry do
        local subEntry = entry[index]
        out[#out + 1] = spaces .. '<key>' .. subEntry.key .. '</key>\n'
        if type(subEntry.value) == 'string' then
            out[#out + 1] = spaces .. '<string>' .. subEntry.value .. '</string>\n'
        elseif type(subEntry.value) == 'number' then
            out[#out + 1] = spaces .. '<integer>' .. subEntry.value .. '</integer>\n'
        elseif type(subEntry.value) == 'boolean' then
            if subEntry.value then
                out[#out + 1] = spaces .. '<true />\n'
            else
                out[#out + 1] = spaces .. '<false />\n'
            end
        elseif type(subEntry.value) == 'table'  and  not getmetatable(subEntry.value) then
            out[#out + 1] = spaces .. '<array>\n'
            local subValue = subEntry.value[1]
            if subValue then
                if type(subValue) == 'table' then
                    for dictIndex = 1, #subEntry.value do
                        local dictEntry = subEntry.value[dictIndex]
                        ExportDict(dictEntry, out, spaces .. '  ')
                    end
                elseif type(subValue) == 'string' then
                    for listIndex = 1, #subEntry.value do
                        local listEntry = subEntry.value[listIndex]
                        out[#out + 1] = spaces .. '  <string>' .. listEntry .. '</string>\n'
                    end
                elseif type(subValue) == 'number' then
                    for listIndex = 1, #subEntry.value do
                        local listEntry = subEntry.value[listIndex]
                        out[#out + 1] = spaces .. '  <integer>' .. tostring(listEntry) .. '</integer>\n'
                    end
                end
            end
            out[#out + 1] = spaces .. '</array>\n'
        elseif type(subEntry.value) == 'table'  and  getmetatable(subEntry.value) == plistDictMetatable then
            ExportDict(subEntry.value, out, spaces)
        end
    end
    out[#out + 1] = origSpaces .. '</dict>\n'
end


function M.dump(dict)
    local out = {}
    out[#out + 1] = '<?xml version="1.0" encoding="utf-8"?>\n'
    out[#out + 1] = '<plist version="1.0">\n'

    if dict then
        ExportDict(dict, out, '  ')
    end

    out[#out + 1] = '</plist>'
    return table.concat(out)
end


function M.write(dict, filename)
    local file = io.open(filename, 'w')
    if not file then return end
    file:write(M.dump(dict))
    file:close()
    return true
end


function M.merge(sourceDict, destinationDict)
    local destinationKeys = {}
    for destinationIndex = 1, #destinationDict do
        local destinationKey = destinationDict[destinationIndex]
        destinationKeys[destinationKey.key] = destinationIndex
    end
    for sourceIndex = 1, #sourceDict do
        local sourceEntry = sourceDict[sourceIndex]
        local destinationIndex = destinationKeys[sourceEntry.key]
        if not destinationIndex then
            destinationIndex = #destinationDict + 1
            rawset(destinationDict, destinationIndex, { key = sourceEntry.key })
        end
        local destinationEntry = destinationDict[destinationIndex]
        local sourceValueType = type(sourceEntry.value)
        if sourceValueType == 'string' then
            destinationEntry.value = sourceEntry.value
        elseif sourceValueType == 'number' then
            destinationEntry.value = sourceEntry.value
        elseif sourceValueType == 'boolean' then
            destinationEntry.value = sourceEntry.value
        elseif sourceValueType == 'table' then
            local sourceArrayValue = sourceEntry.value[1]
            if sourceArrayValue then
                if type(sourceArrayValue) == 'table' then
                    local destinationArray = destinationEntry.value
                    if type(destinationArray) ~= 'table'  or  type(destinationArray[1]) ~= 'table' then
                        destinationArray = M.newarray()
                        destinationEntry.value = destinationArray
                    end
                    for sourceListIndex = 1, #sourceEntry.value do
                        local sourceListEntry = sourceEntry.value[sourceListIndex]
                        if not destinationArray[sourceListIndex] then
                            destinationArray[sourceListIndex] = M.newdict()
                        end

                        M.merge(sourceListEntry, destinationArray[sourceListIndex])
                    end
                elseif type(sourceArrayValue) == 'string' then
                    local destinationArray = {}
                    destinationEntry.value = destinationArray
                    for sourceListIndex = 1, #sourceEntry.value do
                        local sourceListEntry = sourceEntry.value[sourceListIndex]
                        destinationArray[#destinationArray + 1] = sourceListEntry
                    end
                elseif type(sourceArrayValue) == 'number' then
                    local destinationArray = {}
                    destinationEntry.value = destinationArray
                    for sourceListIndex = 1, #sourceEntry.value do
                        local sourceListEntry = sourceEntry.value[sourceListIndex]
                        destinationArray[#destinationArray + 1] = sourceListEntry
                    end
                end
            end
        end
    end
end


function M.newarray()
    return {}
end


function M.newdict()
    return setmetatable({}, plistDictMetatable)
end

return M


--left = plist.read('Info.left.plist')
--right = plist.read('Info.right.plist')
--require 'prettydump'.dumpascii('s:/info.plist.lua', 'dict', left)

--left['CFBundleDisplayName'] = 'My Game!'
--left['NotHereYet'] = 'The string!'

--print(left.UILaunchImages)
--print(left['UILaunchImages'][1].UILaunchImageName)
--print(#left['UILaunchImages'])
--print(plist.dump(left))

--plist.merge(left, right)
--plist.write(right, 'out.plist')

