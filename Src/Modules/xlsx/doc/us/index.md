## Introduction

The module *xlsx* allows read and write access to .xlsx files.



## Example

<pre>
    require 'xlsx'

    local workbook = xlsx.Workbook(filename)
</pre>



-------------------------
## xlsx Reference

### workbook = xlsx.Workbook([filename])

Creates or opens a Workbook object.  Returns a Workbook object.

* `filename` is the name of the .xlsx file to open.




-------------------------
## Workbook Reference

### totalWorksheets = #workbook

Returns the total number of Excel worksheets in the current Excel workbook.



### worksheet = workbook[sheetIndex | name]

Returns an object representing the Excel worksheet specified by `sheetIndex` or `name`.  If no worksheet is found with the given `sheetIndex` or `name`, then `nil` is returned.

* `sheetIndex` is an index in the range of 1 <= `sheetIndex` <= `GetTotalWorksheets` representing the sheet to retrieve.
* `name` is a string representing the sheet name to retrieve.



### workbook:Close()

Closes the current Excel workbook.




### totalWorksheets = workbook:GetTotalWorksheets()

Returns the total number of Excel worksheets in the current Excel workbook.




### worksheet = workbook:GetWorksheet(sheetIndex | name)

Returns an object representing the Excel worksheet specified by `sheetIndex` or `name`.  If no worksheet is found with the given `sheetIndex` or `name`, then `GetWorksheet` returns `nil`.

* `sheetIndex` is an index in the range of 0 <= `sheetIndex` < `GetTotalWorksheets()` representing the sheet to retrieve.
* `name` is either a string or an xls.wchar representing the sheet name to retrieve.




### sheetName = workbook:GetSheetName(sheetIndex)

Returns either the name of the sheet.

* `sheetIndex` is an index in the range of 1 <= `sheetIndex` <= `GetTotalWorksheets()` representing the worksheet to delete.




-------------------------
## Worksheet Reference

### sheetName = worksheet:GetSheetName()

Returns the name of the sheet.




### totalRows = worksheet:GetTotalRows()

Returns the total number of rows in the Excel worksheet.




### totalColumns = worksheet:GetTotalCols()

Returns the total number of columns in the Excel worksheet.



### cell = worksheet:Cell(row, col)

Retrieves the contents of a cell from the Excel worksheet.

Returns the cell if the operation succeeded, `nil` if either the row or column are not in range.

* `row` is a value from 0 to 65535, representing the row in the Excel worksheet to retrieve.
* `col` is a value from 0 to 255, representing the column in the Excel worksheet to retrieve.




### cell = worksheet.COLROW

Retrieves the contents of a cell from the Excel worksheet.

Returns the cell if the operation succeeded, `nil` if either the row or column are not in range.

* `COLROW` is a column and row in Excel format, such as A4 or BD12.





-------------------------
## Cell Reference

### cellType = cell:Type()

Returns one of the following as the type of this Excel cell.

* `cell.UNDEFINED`
* `cell.INT`
* `cell.DOUBLE`
* `cell.STRING`
* `cell.WSTRING`
* `cell.FORMULA`



### cellContent = cell.value

Returns the raw value of the cell.




### cellContent = cell:Get()

If the type of the cell is `cell.INT` or `cell.DOUBLE`, the integer or double content of the cell is returned as a Lua number.  If the type of the cell is `cell.STRING`, the ANSI string content of the cell is returned as a Lua string.  If the type of the cell is `cell.WSTRING`, the Unicode string content of the cell are returned as an `xls.wchar`.  Otherwise, `nil` is returned.



### cellContent = cell:GetInteger()

If the type of the cell is `cell.INT`, the integer content of the cell is returned as a Lua number.  Otherwise, `nil` is returned.



### cellContent = cell:GetDouble()

If the type of the cell is `cell.DOUBLE`, the double content of the cell is returned as a Lua number.  Otherwise, `nil` is returned.



### cellContent = cell:GetString()

If the type of the cell is `cell.STRING`, the ANSI string content of the cell is returned as a Lua string.  Otherwise, `nil` is returned.

