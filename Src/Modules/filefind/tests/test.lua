local filefind = require 'filefind'

require 'ex'

function TestList(expectedList, wildcard)
	local expectedListMap = {}
	for _, name in ipairs(expectedList) do
		expectedListMap[name] = true
	end

	local foundMap = {}
	for handle in filefind.glob(wildcard) do
		foundMap[handle.filename] = true
	end

	local extraList = {}
	for foundName in pairs(foundMap) do
		if not expectedListMap[foundName] then
			extraList[#extraList + 1] = foundName
		end
		expectedListMap[foundName] = nil
	end
	if #extraList > 0 then
		table.sort(extraList)
        print('Files on disk (' .. wildcard .. '):')
        for entry in filefind.glob(wildcard) do
            print('', handle.filename)
        end
		error('These entries should not exist:\n\t\t' .. table.concat(extraList, '\n\t\t'))
	end

	local missingEntries = {}
	for expectedName, value in pairs(expectedListMap) do
		missingEntries[#missingEntries + 1] = expectedName
	end
	if #missingEntries > 0 then
		print('These entries are missing:\n\t\t' .. table.concat(missingEntries, '\n\t\t'))
        print('Files on disk (' .. wildcard .. '):')
        for entry in filefind.glob(wildcard) do
            print('', handle.filename)
        end
        error("error")
	end
end

function io.writefile(filename, buffer)
	os.mkdir(filename)
	local file = io.open(filename, 'wb')
	file:write(buffer)
	file:close()
end

--for handle in filefind.glob('**') do
--	print(handle)
--end

-------------------------------------------------------------------------------
iterfunc = filefind.glob('*')
assert(type(iterfunc) == 'function')

handle = iterfunc()
assert(type(handle) == 'userdata')
assert(type(handle.filename) == 'string')
assert(type(handle.creation_time) == 'number')
assert(type(handle.access_time) == 'number')
assert(type(handle.write_time) == 'number')
assert(type(handle.creation_FILETIME) == 'table')
assert(type(handle.creation_FILETIME[1]) == 'number')
assert(type(handle.creation_FILETIME[2]) == 'number')
assert(type(handle.access_FILETIME) == 'table')
assert(type(handle.access_FILETIME[1]) == 'number')
assert(type(handle.access_FILETIME[2]) == 'number')
assert(type(handle.write_FILETIME) == 'table')
assert(type(handle.write_FILETIME[1]) == 'number')
assert(type(handle.write_FILETIME[2]) == 'number')
assert(type(handle.size) == 'number')
assert(type(handle.is_directory) == 'boolean')
assert(handle.is_directory == false)
assert(type(handle.is_readonly) == 'boolean')

local entryTable = handle.table
assert(entryTable.filename == handle.filename)
assert(entryTable.creation_time == handle.creation_time)
assert(entryTable.access_time == handle.access_time)
assert(entryTable.write_time == handle.write_time)
assert(entryTable.creation_FILETIME[1] == handle.creation_FILETIME[1])
assert(entryTable.creation_FILETIME[2] == handle.creation_FILETIME[2])
assert(entryTable.access_FILETIME[1] == handle.access_FILETIME[1])
assert(entryTable.access_FILETIME[2] == handle.access_FILETIME[2])
assert(entryTable.write_FILETIME[1] == handle.write_FILETIME[1])
assert(entryTable.write_FILETIME[2] == handle.write_FILETIME[2])
assert(entryTable.size == handle.size)
assert(entryTable.is_directory == handle.is_directory)
assert(entryTable.is_readonly == handle.is_readonly)

-------------------------------------------------------------------------------
os.remove('a/')
os.remove('b/')
TestList( { 'test.lua' }, '**' )

-------------------------------------------------------------------------------
io.writefile('a/.svn/entries', 'junk')
io.writefile('a/subdira/.svn/entries', 'junk')
io.writefile('a/subdira/.svn/index', 'junk')
io.writefile('b/.svn/entries', 'junk')
io.writefile('a/filea.txt', 'filea')
io.writefile('a/subdira/fileb.dat', 'fileb')
io.writefile('a/subdira/subdirb/subfileb.xyz', 'subfileb')
io.writefile('a/subdira/subdirb/subfilec.zyx', 'subfilec')
io.writefile('a/filec.txt', 'filec')
io.writefile('b/filed.txt', 'filed')
io.writefile('b/35/960/file.dat', 'file')
io.writefile('b/73/12/file.dat', 'file')


-------------------------------------------------------------------------------
TestList(
	{
		'test.lua',
	}, '*' )


-------------------------------------------------------------------------------
TestList(
	{
		'test.lua',
	}, '*.*' )


-------------------------------------------------------------------------------
TestList(
	{
		'test.lua',
	}, '*e*' )


-------------------------------------------------------------------------------
TestList(
	{
	}, '*z*' )


-------------------------------------------------------------------------------
TestList(
	{
		'test.lua',
		'a/filea.txt',
		'a/filec.txt',
		'a/.svn/entries',
		'a/subdira/fileb.dat',
		'a/subdira/.svn/entries',
		'a/subdira/.svn/index',
		'a/subdira/subdirb/subfileb.xyz',
		'a/subdira/subdirb/subfilec.zyx',
		'b/filed.txt',
		'b/.svn/entries',
		'b/35/960/file.dat',
		'b/73/12/file.dat',
	}, '**' )


-------------------------------------------------------------------------------
TestList(
	{
		'a/subdira/fileb.dat',
		'b/35/960/file.dat',
		'b/73/12/file.dat',
	}, '**.dat' )


-------------------------------------------------------------------------------
TestList(
	{
		'a/subdira/fileb.dat',
		'b/35/960/file.dat',
		'b/73/12/file.dat',
	}, '**/*.dat' )


-------------------------------------------------------------------------------
TestList(
	{
		'b/35/960/file.dat',
		'b/73/12/file.dat',
	}, 'b/*/*/file.dat' )


-------------------------------------------------------------------------------
TestList(
	{
		'a/filea.txt',
		'a/filec.txt',
		'a/subdira/fileb.dat',
		'b/filed.txt',
		'b/35/960/file.dat',
		'b/73/12/file.dat',
	}, '**t' )


-------------------------------------------------------------------------------
TestList(
	{
		'a/filea.txt',
		'a/filec.txt',
		'a/subdira/fileb.dat',
		'b/filed.txt',
		'b/35/960/file.dat',
		'b/73/12/file.dat',
	}, '**/*t' )


-------------------------------------------------------------------------------
TestList(
	{
		'a/',
		'a/.svn/',
		'a/subdira/',
		'a/subdira/.svn/',
		'a/subdira/subdirb/',
		'b/',
		'b/.svn/',
		'b/35/',
		'b/35/960/',
		'b/73/',
		'b/73/12/',
	}, '**/' )


-------------------------------------------------------------------------------
TestList(
	{
		'b/',
		'a/subdira/subdirb/',
	}, '**b/' )


-------------------------------------------------------------------------------
TestList(
	{
		'b/',
		'a/subdira/',
		'a/subdira/subdirb/',
	}, '**b*/' )


-------------------------------------------------------------------------------
TestList(
	{
		'a/',
		'b/',
		'test.lua',
		'a/.svn/',
		'a/filea.txt',
		'a/filec.txt',
		'a/subdira/',
		'a/.svn/entries',
		'a/subdira/.svn/',
		'a/subdira/fileb.dat',
		'a/subdira/subdirb/',
		'a/subdira/.svn/entries',
		'a/subdira/.svn/index',
		'a/subdira/subdirb/subfileb.xyz',
		'a/subdira/subdirb/subfilec.zyx',
		'b/.svn/',
		'b/35/',
		'b/35/960/',
		'b/73/',
		'b/73/12/',
		'b/filed.txt',
		'b/.svn/entries',
		'b/35/960/file.dat',
		'b/73/12/file.dat',
	}, '**@*' )


-------------------------------------------------------------------------------
TestList(
	{
		'test.lua',
	}, '**.lua' )


-------------------------------------------------------------------------------
TestList(
	{
		'test.lua',
	}, '**@=*.lua' )


-------------------------------------------------------------------------------
TestList(
	{
		'test.lua',
	}, '**@=*.lua' )


-------------------------------------------------------------------------------
TestList(
	{
		'test.lua',
		'a/filea.txt',
		'a/filec.txt',
		'b/filed.txt',
	}, '**@=*.lua@=**.txt' )


-------------------------------------------------------------------------------
TestList(
	{
		'test.lua',
		'a/filea.txt',
		'a/filec.txt',
		'b/filed.txt',
	}, '**@$SKIPME$@=*.lua@=**.txt' )


-------------------------------------------------------------------------------
TestList({
		'test.lua',
		'a/filea.txt',
		'a/filec.txt',
		'a/subdira/fileb.dat',
		'a/subdira/subdirb/subfileb.xyz',
		'a/subdira/subdirb/subfilec.zyx',
		'b/filed.txt',
		'b/35/960/file.dat',
		'b/73/12/file.dat',
	}, '**@-**/.svn/' )


-------------------------------------------------------------------------------
TestList({
		'a/',
		'b/',
		'a/subdira/',
		'a/subdira/subdirb/',
		'b/35/',
		'b/35/960/',
		'b/73/',
		'b/73/12/',
	}, '**/@-**/.svn/' )


-------------------------------------------------------------------------------
TestList({
		'a/',
		'b/',
		'a/.svn/',
		'a/subdira/',
		'a/subdira/.svn/',
		'a/subdira/subdirb/',
		'b/.svn/',
		'b/35/',
		'b/35/960/',
		'b/73/',
		'b/73/12/',
	}, '**/@-.svn' )


-------------------------------------------------------------------------------
TestList({
		'a/',
		'b/',
		'a/.svn/',
		'b/.svn/',
		'b/35/',
		'b/35/960/',
		'b/73/',
		'b/73/12/',
	}, '**/@-**/s*/' )


-------------------------------------------------------------------------------
TestList({
		'a/',
		'b/',
		'a/.svn/',
		'a/subdira/',
		'a/subdira/.svn/',
		'a/subdira/subdirb/',
		'b/.svn/',
		'b/35/',
		'b/35/960/',
		'b/73/',
		'b/73/12/',
	}, '**/@-*s*/' )


-------------------------------------------------------------------------------
TestList({
		'a/',
		'b/',
		'b/35/',
		'b/35/960/',
		'b/73/',
		'b/73/12/',
	}, '**/@-**s*/' )


-------------------------------------------------------------------------------
TestList(
	{
		'a/',
		'b/',
		'test.lua',
	}, '*@*' )


-------------------------------------------------------------------------------
TestList(
	{
		'a/',
		'test.lua',
		'b/',
		'a/filea.txt',
		'a/filec.txt',
		'a/subdira/',
		'a/subdira/fileb.dat',
		'a/subdira/subdirb/',
		'a/subdira/subdirb/subfileb.xyz',
		'a/subdira/subdirb/subfilec.zyx',
		'b/35/',
		'b/35/960/',
		'b/73/',
		'b/73/12/',
		'b/filed.txt',
		'b/35/960/file.dat',
		'b/73/12/file.dat',
	}, '**@*@-**n/' )


-------------------------------------------------------------------------------
TestList(
	{
		'a/',
		'test.lua',
		'b/',
		'a/filea.txt',
		'a/filec.txt',
		'a/subdira/',
		'a/subdira/subdirb/',
		'a/subdira/subdirb/subfileb.xyz',
		'a/subdira/subdirb/subfilec.zyx',
		'b/35/',
		'b/35/960/',
		'b/73/',
		'b/73/12/',
	}, '**@*@-**n/@-**/*d*' )




-------------------------------------------------------------------------------
os.remove('a/')
os.remove('b/')




-------------------------------------------------------------------------------
function test(a, b)
	if a ~= b then
		print('Actual:', a)
		print('Expected:', b)
		assert()
	end
end

test(filefind.pattern_match('**b*.zip', 'buildtest.zip'), true)
test(filefind.pattern_match('**b*.zip', 'subdira/subdirb/file1.zip'), false)
test(filefind.pattern_match('**/*', 'subdira/subdirb/abcdefg.txt'), true)
test(filefind.pattern_match('**', 'subdira/subdirb/abcdefg.txt'), true)
test(filefind.pattern_match('', ''), true)
test(filefind.pattern_match('*', ''), true)
test(filefind.pattern_match('*.*', ''), false)
test(filefind.pattern_match('a*', ''), false)
test(filefind.pattern_match('a*', 'abcdefg.txt'), true)
test(filefind.pattern_match('a*b*c?e*', 'abcdefg.txt'), true)
test(filefind.pattern_match('a*b*c?f*', 'abcdefg.txt'), false)
test(filefind.pattern_match('*.', 'abcdefg.txt'), false)
test(filefind.pattern_match('*.t', 'abcdefg.txt'), false)
test(filefind.pattern_match('*.t*', 'abcdefg.txt'), true)
test(filefind.pattern_match('*.t', 'abcdefg.txt'), false)
test(filefind.pattern_match('*.*t', 'abcdefg.txt'), true)
test(filefind.pattern_match('*.*x', 'abcdefg.txt'), false)
test(filefind.pattern_match('*.txt', 'abcdefg.txt'), true)

test(filefind.pattern_match('*', 'subdira/abcdefg.txt'), false)
test(filefind.pattern_match('*/*', 'subdira/abcdefg.txt'), true)
test(filefind.pattern_match('*/*.dat', 'subdira/abcdefg.txt'), false)
test(filefind.pattern_match('*/*.txt', 'subdira/abcdefg.txt'), true)

test(filefind.pattern_match('*', 'subdira/subdirb/abcdefg.txt'), false)
test(filefind.pattern_match('*/*', 'subdira/subdirb/abcdefg.txt'), false)
test(filefind.pattern_match('*/*/*', 'subdira/subdirb/abcdefg.txt'), true)
test(filefind.pattern_match('*/*/*.dat', 'subdira/subdirb/abcdefg.txt'), false)
test(filefind.pattern_match('*/*/*.txt', 'subdira/subdirb/abcdefg.txt'), true)
test(filefind.pattern_match('**', 'subdira/subdirb/abcdefg.txt'), true)
test(filefind.pattern_match('**/*', 'subdira/subdirb/abcdefg.txt'), true)
test(filefind.pattern_match('**/*.dat', 'subdira/subdirb/abcdefg.txt'), false)
test(filefind.pattern_match('**/*.txt', 'subdira/subdirb/abcdefg.txt'), true)
test(filefind.pattern_match('s**/*.txt', 'subdira/subdirb/abcdefg.txt'), true)
test(filefind.pattern_match('*a**/*.txt', 'subdira/subdirb/abcdefg.txt'), true)
test(filefind.pattern_match('t**/*.txt', 'subdira/subdirb/abcdefg.txt'), false)
--test(filefind.canonicalize_wild_pattern('**b*.zip'), '**/*b*.zip')
test(filefind.pattern_match('**/*b*.zip', 'subdira/subdirb/file1.zip'), false)


print("* Testing complete!")

