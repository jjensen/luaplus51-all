local xlsx = require 'xlsx'

-- Load a workbook with one sheet, display its contents and
-- save into another file.
--if Tilde then Tilde('disable') end
local workbook = xlsx.Workbook(arg[1])
if Tilde then Tilde('enable') end
for sheetIndex = 1, workbook:GetTotalWorksheets() do
    local sheet1 = workbook[sheetIndex]
    local maxRows = sheet1:GetTotalRows()
    local maxCols = sheet1:GetTotalCols()
    print(sheetIndex, "Dimension of " .. sheet1:GetSheetName() .. " (" .. maxRows .. ", " .. maxCols .. ")")

    print()
    for c = 0, maxCols - 1 do io.write(("%10d|"):format(c + 1)) end
    io.write('\n')

    for r = 0, maxRows - 1 do
        io.write(("%10d|"):format(r + 1))
        for c = 0, maxCols - 1 do
            local cell = sheet1:Cell(r, c)
            if cell then
                local cellType = cell:Type()
                if cellType == cell.UNDEFINED then
                    io.write('          |')

                elseif cellType == cell.BOOLEAN then
                    io.write(("%10s|"):format(cell:GetBoolean()  and  'TRUE'  or  'FALSE'))

                elseif cellType == cell.INT then
                    io.write(("%10d|"):format(cell:GetInteger()))

                elseif cellType == cell.DOUBLE then
                    io.write(("%10.6g|"):format(cell:GetDouble()))

                elseif cellType == cell.STRING then
                    io.write(("%10s|"):format(cell:GetString()))

                elseif cellType == cell.WSTRING then
                    io.write(("%10s|"):format(tostring(cell:GetWString())))
                end
            else
                io.write('          |')
            end
        end
        io.write('\n')
    end
    io.write('\n')
end

