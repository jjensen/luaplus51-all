## Introduction

The module *xls* allows high performance read and write access to .xls files.



## Example

<pre>
    require 'xls'

    local doc = xls.BasicExcel()
</pre>



-------------------------
## xls Reference

### workbook = xls.Workbook([filename])

Creates or opens a Workbook object.  Returns a Workbook object.

* `filename` is the optional name of the .xls file to open.  If no `filename` is given, an empty Workbook is created.




-------------------------
## Workbook Reference

### workbook:New(sheets [ = 3])

Creates a new Excel workbook with a given number of worksheets.

* `sheets` is the number of worksheets to create within the workbook.  The minimum is 1.  The default is 3.


### loaded = workbook:Load(filename)

Loads an Excel workbook from a file.

Returns `true` if successful, `false` otherwise.

* `filename` is the path to the .xls file to open.



### saved = workbook:Save()

Saves current Excel workbook to the opened file.

Returns `true` if successful, `false` if unsuccessful.




### saved = workbook:SaveAs(filename)

Loads an Excel workbook from a file.

Returns `true` if successful, `false` otherwise.

* `filename` is the path to the .xls file to save.  `filename` may be a regular string or an `xls.wchar`.



### workbook:Close()

Closes the current Excel workbook.




### totalWorksheets = workbook:GetTotalWorksheets()

Returns the total number of Excel worksheets in the current Excel workbook.




### worksheet = workbook:GetWorksheet(sheetIndex | name)

Returns an object representing the Excel worksheet specified by `sheetIndex` or `name`.  If no worksheet is found with the given `sheetIndex` or `name`, then `GetWorksheet` returns `nil`.

* `sheetIndex` is an index in the range of 0 <= `sheetIndex` < `GetTotalWorksheets()` representing the sheet to retrieve.
* `name` is either a string or an xls.wchar representing the sheet name to retrieve.




### worksheet = workbook:AddWorksheet([sheetIndex] | [name [, sheetIndex]])

Adds a new worksheet to the workbook.  If no `sheetIndex` or `name` is specified, a new worksheet is added to the last position.  

Returns an Excel worksheet specified by `sheetIndex` or `name`.

* `sheetIndex` is an index in the range of -1 <= `sheetIndex` < `GetTotalWorksheets()`.  If `-1` is specified, the new worksheet is added to the last position.  The name given to the new worksheet is `SheetX`, where `X` is a number which starts from 1.
* `name` is either a string or an xls.wchar that will become the name of the worksheet being added.




### deleted = workbook:DeleteWorksheet(sheetIndex | name)

Deletes the specified Excel worksheet from the workbook.

Returns `true` if the worksheet was deleted, `false` otherwise.

* `sheetIndex` is an index in the range of 0 <= `sheetIndex` < `GetTotalWorksheets()` representing the worksheet to delete.
* `name` is either a string or an xls.wchar representing the sheet name to delete.




### ansiSheetName = workbook:GetAnsiSheetName(sheetIndex)

If possible, returns the ANSI name of the sheet.  Otherwise, returns `nil` if the name is Unicode.

* `sheetIndex` is an index in the range of 0 <= `sheetIndex` < `GetTotalWorksheets()` representing the worksheet to delete.




### unicodeSheetName = workbook:GetUnicodeSheetName(sheetIndex)

If possible, returns the Unicode name of the sheet formatted as an `xls.wchar`.  Otherwise, returns `nil` if the name is Unicode.




### sheetName = workbook:GetSheetName(sheetIndex)

Returns either the ANSI name of the sheet or the Unicode name of the sheet as an `xls.wchar`.

* `sheetIndex` is an index in the range of 0 <= `sheetIndex` < `GetTotalWorksheets()` representing the worksheet to delete.




### renamed = workbook:RenameWorksheet(sheetIndex | from, to)

Renames an Excel worksheet at `sheetIndex` or `from` to the name specified by `to`.

Returns `true` if the operation succeeded, `false` otherwise.


* `sheetIndex` is an index in the range of 0 <= `sheetIndex` < `GetTotalWorksheets()` representing the worksheet to delete.
* `from` is either a string or `xls.wchar` name of the worksheet to rename.
* `to` is either a string or `xls.wchar` name, where the type must match that of `from` if `from` is specified, of the worksheet to rename.



