local ziparchive = require 'ziparchive'
local xmlize = require 'xmlize'

local M = {}

local colRowPattern = "([a-zA-Z]*)(%d*)"

local function _xlsx_readdocument(xlsx, documentName)
    local file = xlsx.archive:fileopen(documentName)
    if not file then return end
    local buffer = xlsx.archive:fileread(file)
    xlsx.archive:fileclose(file)
    return xmlize.luaize(buffer)
end

local __cellMetatable = {
    UNDEFINED = 0,
    INT = 1,
    DOUBLE = 2,
    STRING = 3,
    WSTRING = 4,
    FORMULA = 5,
	BOOLEAN = 6,

    Get = function(self)
        return self.value
    end,

    GetBoolean = function(self)
        return self.value
    end,

    GetInteger = function(self)
        return self.value
    end,

    GetDouble = function(self)
        return self.value
    end,

    GetString = function(self)
        return self.value
    end,
}

__cellMetatable.__index = __cellMetatable

function __cellMetatable:Type()
    return self.type
end

local function Cell(rowNum, colNum, value, type, formula)
    return setmetatable({
        row = tonumber(rowNum),
        column = colNum,
        value = value,
        type = type  or  __cellMetatable.UNDEFINED,
        formula = formula,
    }, __cellMetatable)
end

local __colTypeTranslator = {
    b = __cellMetatable.BOOLEAN,
    s = __cellMetatable.STRING,
}

local __sheetMetatable = {
    __load = function(self)
        local sheetDoc = _xlsx_readdocument(self.workbook, ("xl/worksheets/sheet%d.xml"):format(self.id))
        local sheetData = sheetDoc.worksheet[1]['#'].sheetData
        local rows = {}
        local columns = {}
        if sheetData[1]['#'].row then
            for _, rowNode in ipairs(sheetData[1]['#'].row) do
                local rowNum = tonumber(rowNode['@'].r)
                if not rows[rowNum] then
                    rows[rowNum] = {}
                end
                if rowNode['#'].c then
                    for _, columnNode in ipairs(rowNode['#'].c) do
                        -- Generate the proper column index.
                        local cellId = columnNode['@'].r
                        local colLetters = cellId:match(colRowPattern)
                        local colNum = 0
                        if colLetters then
                            local index = 1
                            repeat
                                colNum = colNum * 26
                                colNum = colNum + colLetters:byte(index) - ('A'):byte(1) + 1
                                index = index + 1
                            until index > #colLetters
                        end

                        local colType = columnNode['@'].t

                        local data
                        if columnNode['#'].v then
                            data = columnNode['#'].v[1]['#']
                            if colType == 's' then
                                colType = __cellMetatable.STRING
                                data = self.workbook.sharedStrings[tonumber(data) + 1]
                            elseif colType == 'str' then
                                colType = __cellMetatable.STRING
                            elseif colType == 'b' then
                                colType = __cellMetatable.BOOLEAN
                                data = data == '1'
                            elseif columnNode['@'].s then
                                local cellS = tonumber(columnNode['@'].s)
                                local numberStyle = self.workbook.styles.cellXfs[cellS - 1].numFmtId
                                if not numberStyle then
                                    numberStyle = 0
                                end
                                if numberStyle == 0  or  numberStyle == 1 then
                                    colType = __cellMetatable.INT
                                else
                                    colType = __cellMetatable.DOUBLE
                                end
                                data = tonumber(data)
                            end

                            --local formula
                            --if columnNode['#'].f then
                                --assert()
                            --end

                        else
                            colType = __colTypeTranslator[colType]
                        end

                        if not columns[colNum] then
                            columns[colNum] = {}
                        end
                        local cell = Cell(rowNum, colNum, data, colType, formula)
                        table.insert(rows[rowNum], cell)
                        table.insert(columns[colNum], cell)
                        self.__cells[cellId] = cell
                    end
                end
            end
        end
        self.__rows = rows
        self.__cols = columns
        self.loaded = true
    end,

    rows = function(self)
        if not self.loaded then
            self.__load()
        end
        return self.__rows
    end,

    cols = function(self)
        if not self.loaded then
            self.__load()
        end
        return self.__cols
    end,

    GetAnsiSheetName = function(self)
        return self.name
    end,

    GetUnicodeSheetName = function(self)
        return self.name
    end,

    GetSheetName = function(self)
        return self.name
    end,

    GetTotalRows = function(self)
        return #self.__rows
    end,

    GetTotalCols = function(self)
        return #self.__cols
    end,

    Cell = function(self, row, col)
        local key = ''
        local extraColIndex = math.floor(col / 26)
        if extraColIndex > 0 then
            key = string.char(string.byte('A') + (extraColIndex - 1))
        end
        key = key .. string.char(string.byte('A') + (col % 26))
        key = key .. (row + 1)
        return self.__cells[key]
    end,

    __tostring = function(self)
        return "xlsx.Sheet " .. self.name
    end
}

__sheetMetatable.__index = function(self, key)
    local value = __sheetMetatable[key]
    if value then return value end
    return self.__cells[key]
end


function Sheet(workbook, id, name)
    local self = {}
    self.workbook = workbook
    self.id = id
    self.name = name
    self.loaded = false
    self.__cells = {}
    self.__cols = {}
    self.__rows = {}
    setmetatable(self, __sheetMetatable)
    return self
end


local __workbookMetatableMembers = {
    GetTotalWorksheets = function(self)
        return #self.__sheets
    end,

    GetWorksheet = function(self, key)
        return self.__sheets[key]
    end,

    GetAnsiSheetName = function(self, key)
        return self:GetWorksheet(key).name
    end,

    GetUnicodeSheetName = function(self, key)
        return self:GetWorksheet(key).name
    end,

    GetSheetName = function(self, key)
        return self:GetWorksheet(key).name
    end,

    Sheets = function(self)
        local i = 0
        return function()
            i = i + 1
            return self.__sheets[i]
        end
    end
}


local __workbookMetatable = {
    __len = function(self)
        return #self.__sheets
    end,

    __index = function(self, key)
        local value = __workbookMetatableMembers[key]
        if value then return value end
        return self.__sheets[key]
    end
}


function M.Workbook(filename)
    local self = {}

    self.archive = ziparchive.open(filename)

    local sharedStringsXml = _xlsx_readdocument(self, 'xl/sharedstrings.xml')
    self.sharedStrings = {}
    if sharedStringsXml then
        for _, str in ipairs(sharedStringsXml.sst[1]['#'].si) do
            if str['#'].r then
                local concatenatedString = {}
                for _, rstr in ipairs(str['#'].r) do
                    local t = rstr['#'].t[1]['#']
                    if type(t) == 'string' then
                        concatenatedString[#concatenatedString + 1] = rstr['#'].t[1]['#']
                    end
                end
                concatenatedString = table.concat(concatenatedString)
                self.sharedStrings[#self.sharedStrings + 1] = concatenatedString
            else
                self.sharedStrings[#self.sharedStrings + 1] = str['#'].t[1]['#']
            end
        end
    end

    local stylesXml = _xlsx_readdocument(self, 'xl/styles.xml')
    self.styles = {}
    local cellXfs = {}
    self.styles.cellXfs = cellXfs
    if stylesXml then
        for _, xfXml in ipairs(stylesXml.styleSheet[1]['#'].cellXfs[1]['#'].xf) do
            local xf = {}
            local numFmtId = xfXml['@'].numFmtId
            if numFmtId then
                xf.numFmtId = tonumber(numFmtId)
            end
            cellXfs[#cellXfs + 1] = xf
        end
    end

    self.workbookDoc = _xlsx_readdocument(self, 'xl/workbook.xml')
    local sheets = self.workbookDoc.workbook[1]['#'].sheets
    self.__sheets = {}
    local id = 1
    for _, sheetNode in ipairs(sheets[1]['#'].sheet) do
        local name = sheetNode['@'].name
        local sheet = Sheet(self, id, name)
        sheet:__load()
        self.__sheets[id] = sheet
        self.__sheets[name] = sheet
        id = id + 1
    end
    setmetatable(self, __workbookMetatable)
    return self
end

return M

--[[
xlsx = M

local workbook = xlsx.Workbook('Book1.xlsx')
print("Book")
local sheet = workbook[1]
print(sheet:Cell(0, 52))
cell = sheet:Cell(0, 52)
print(cell.value)
print(cell:Get())
print(workbook:GetTotalWorksheets())
print(sheet:rows())
print(sheet:cols())
print(sheet.B1)


--]]

-- vim: set tabstop=4 expandtab:
