local path = require 'ex.path'

scriptPath = ((debug.getinfo(1, "S").source:match("@(.+)[\\/]") or '.') .. '\\'):gsub('\\', '/'):lower()

function test(a, b)
	if a ~= b then
		print('Actual:', a)
		print('Expected:', b)
		assert()
	end
end

test(path.add_extension('', 'ext'), '.ext')
test(path.add_extension(' ', ' ext'), ' . ext')
test(path.add_extension('', '.ext'), '.ext')
test(path.add_extension('hello', 'ext'), 'hello.ext')
test(path.add_extension('hello', '.ext'), 'hello.ext')
test(path.add_extension('hello.txt', 'ext'), 'hello.txt.ext')
test(path.add_extension('hello.txt', '.ext'), 'hello.txt.ext')


test(path.add_slash(''), '/')
test(path.add_slash(' '), ' /')
test(path.add_slash('hello'), 'hello/')
test(path.add_slash(' hello'), ' hello/')
test(path.add_slash(' hello '), ' hello /')
test(path.add_slash('hello/'), 'hello/')


test(path.append('', 'filename.txt'), 'filename.txt')
test(path.append('', 'dir', 'filename.txt'), 'dir/filename.txt')
test(path.append('', 'dirA', '', 'dirB', 'filename.txt'), 'dirA/dirB/filename.txt')
test(path.append('..', 'filename.txt'), '../filename.txt')
test(path.append('root', 'filename.txt'), 'root/filename.txt')
test(path.append('root', 'dir', 'filename.txt'), 'root/dir/filename.txt')
test(path.append('root', 'dirA', '', 'dirB', 'filename.txt'), 'root/dirA/dirB/filename.txt')
test(path.append('root', 'dirA', '', 'dirB', '..', 'filename.txt'), 'root/dirA/dirB/../filename.txt')
test(path.append('root', 'dirA', '', '/dirB', '..', 'filename.txt'), '/dirB/../filename.txt')


test(path.combine('', 'filename.txt'), 'filename.txt')
test(path.combine('', 'dir', 'filename.txt'), 'dir/filename.txt')
test(path.combine('', 'dirA', '', 'dirB', 'filename.txt'), 'dirA/dirB/filename.txt')
test(path.combine('..', 'filename.txt'), '../filename.txt')
test(path.combine('root', 'filename.txt'), 'root/filename.txt')
test(path.combine('root', 'dir', 'filename.txt'), 'root/dir/filename.txt')
test(path.combine('root', 'dirA', '', 'dirB', 'filename.txt'), 'root/dirA/dirB/filename.txt')
test(path.combine('root', 'dirA', '', 'dirB', '..', 'filename.txt'), 'root/dirA/filename.txt')
test(path.combine('c:/test/test2', '..', '.'), 'c:/test/')
test(path.combine('c:/test/test2', '..', '.', 'filename.txt'), 'c:/test/filename.txt')
test(path.combine('c:/test/test2', '..'), 'c:/test/')
test(path.combine('c:/test/test2', '..', '..'), 'c:/')
test(path.combine('c:/test/test2', '..', '..', '..'), 'c:/')


test(path.escape(''), '')
test(path.escape('filename.txt'), 'filename.txt')
if os.getenv("OS") == "Windows_NT" then
	test(path.escape(' '), '" "')
	test(path.escape('file name.txt'), '"file name.txt"')
else
	test(path.escape(' '), '\\ ')
	test(path.escape('file name.txt'), 'file\\ name.txt')
end


test(path.exists(scriptPath .. 'pathtests.lua'), true)
test(path.exists(scriptPath .. 'pathtests2.lua'), false)


test(path.find_extension(''), nil)
test(path.find_extension('filename'), nil)
test(path.find_extension('.lua'), 1)
test(path.find_extension('pathtests.lua'), 10)
test(path.find_extension('pathtests'), nil)


test(path.find_filename('pathtests.lua'), 1)
test(path.find_filename('/pathtests'), 2)
test(path.find_filename('c:/pathtests'), 4)
test(path.find_filename('c:pathtests'), 3)


test(path.get_extension(''), '')
test(path.get_extension('filename'), '')
test(path.get_extension('filename.ext'), '.ext')
test(path.get_extension('filename.txt.ext'), '.ext')


test(path.get_filename(''), '')
test(path.get_filename('filename'), 'filename')
test(path.get_filename('filename.ext'), 'filename.ext')
test(path.get_filename('c:/directory/filename.ext'), 'filename.ext')
test(path.get_filename('c:/directory/'), '')


test(path.is_directory(''), false)
test(path.is_directory(scriptPath .. 'pathtests.lua'), false)
test(path.is_directory('.'), true)
test(path.is_directory('..'), true)
test(path.is_directory(scriptPath .. '../tests'), true)
test(path.is_directory(scriptPath .. '../tests/'), true)


test(path.is_file(''), nil)
test(path.is_file(scriptPath .. 'pathtests.lua'), true)
test(path.is_file('.'), false)
test(path.is_file('..'), false)
test(path.is_file(scriptPath .. '../tests'), false)
test(path.is_file(scriptPath .. '../tests/'), nil)


test(path.is_relative(''), true)
test(path.is_relative('filename.ext'), true)
test(path.is_relative('/filename.ext'), false)
test(path.is_relative('c:/filename.ext'), false)


test(path.is_root(''), false)
test(path.is_root('filename.ext'), false)
test(path.is_root('/filename.ext'), true)
test(path.is_root('c:/filename.ext'), true)


test(path.is_unc(''), false)
test(path.is_unc('filename.ext'), false)
test(path.is_unc('/filename.ext'), false)
test(path.is_unc('c:/filename.ext'), false)
test(path.is_unc('\\\\share'), true)
test(path.is_unc('//share'), true)


local cwd = path.getcwd():gsub('\\', '/')
test(path.make_absolute(''), cwd)
test(path.make_absolute('.'), cwd .. '/')
test(path.make_absolute('..'), cwd:match('(.+)/') .. '/')
test(path.make_absolute('abc'), cwd .. '/abc')


test(path.make_backslash(''), '')
test(path.make_backslash(' '), ' ')
test(path.make_backslash('\\\\abc'), '\\\\abc')
test(path.make_backslash('//abc'), '\\\\abc')
test(path.make_backslash('c:/abc/def/'), 'c:\\abc\\def\\')


test(path.make_slash(''), '')
test(path.make_slash(' '), ' ')
test(path.make_slash('\\\\abc'), '//abc')
test(path.make_slash('//abc'), '//abc')
test(path.make_slash('c:\\abc\\def\\'), 'c:/abc/def/')


do
	os.remove('out.dat')

	local file = io.open('out.dat', 'wb')
	file:write('junk')
	file:close()

	test(path.is_writable('out.dat'), true)
	path.chmod('out.dat', 'r')
	test(path.is_writable('out.dat'), false)
	path.make_writable('out.dat')
	test(path.is_writable('out.dat'), true)

	os.remove('out.dat')
end	


test(path.match('', ''), true)
test(path.match('', '*'), true)
test(path.match('', '*.*'), false)
test(path.match('', 'a*'), false)
test(path.match('abcdefg.txt', 'a*'), true)
test(path.match('abcdefg.txt', 'a*b*c?e*'), true)
test(path.match('abcdefg.txt', 'a*b*c?f*'), false)
test(path.match('abcdefg.txt', '*.'), false)
test(path.match('abcdefg.txt', '*.t'), false)
test(path.match('abcdefg.txt', '*.t*'), true)
test(path.match('abcdefg.txt', '*.t'), false)
test(path.match('abcdefg.txt', '*.*t'), true)
test(path.match('abcdefg.txt', '*.*x'), false)
test(path.match('abcdefg.txt', '*.txt'), true)


test(path.remove_directory(''), '')
test(path.remove_directory(' \t'), ' \t')
test(path.remove_directory('abc'), 'abc')
test(path.remove_directory('/abc.'), 'abc.')
test(path.remove_directory('/dir/abc.'), 'abc.')
test(path.remove_directory('c:/abc.'), 'abc.')
test(path.remove_directory('c:/dir/abc'), 'abc')


test(path.remove_extension(''), '')
test(path.remove_extension(' \t'), ' \t')
test(path.remove_extension('abc'), 'abc')
test(path.remove_extension('abc.'), 'abc')
test(path.remove_extension('abc.txt'), 'abc')
test(path.remove_extension('abc.txt.dat'), 'abc.txt')


test(path.remove_filename(''), '')
test(path.remove_filename(' \t'), '')
test(path.remove_filename('abc'), '')
test(path.remove_filename('/abc.'), '/')
test(path.remove_filename('/dir/abc.'), '/dir/')
test(path.remove_filename('c:/abc.'), 'c:/')
test(path.remove_filename('c:/dir/abc'), 'c:/dir/')


test(path.remove_slash(''), '')
test(path.remove_slash(' \t'), ' \t')
test(path.remove_slash('abc'), 'abc')
test(path.remove_slash('abc/'), 'abc')


test(path.replace_extension('', 'ext'), '.ext')
test(path.replace_extension('', '.ext'), '.ext')
test(path.replace_extension('hello', 'ext'), 'hello.ext')
test(path.replace_extension('hello', '.ext'), 'hello.ext')
test(path.replace_extension('hello.txt', 'ext'), 'hello.ext')
test(path.replace_extension('hello.txt', '.ext'), 'hello.ext')
test(path.replace_extension('hello.txt.dat', 'ext'), 'hello.txt.ext')
test(path.replace_extension('hello.txt.dat', '.ext'), 'hello.txt.ext')


test(path.simplify(''), '')
test(path.simplify('abc'), 'abc')
test(path.simplify('.abc'), '.abc')
test(path.simplify('./abc'), 'abc')
test(path.simplify('..abc'), '..abc')
test(path.simplify('../abc'), '../abc')
test(path.simplify('abc/////def'), 'abc/def')
test(path.simplify('abc/././././def'), 'abc/def')
test(path.simplify('c:/somedir/.././././def'), 'c:/def')
test(path.simplify('abc/.././././def'), 'def')
test(path.simplify('abc/ABC/../../../../def'), 'def')
test(path.simplify('c:\\abc\\ABC\\../..\\../..\\def'), 'c:/def')
test(path.simplify('\\\\uncserver\\pathA\\file.txt'), '\\\\uncserver/pathA/file.txt')


function compare_split(inpath, expectedDirname, expectedFilename)
	local actualDirname, actualFilename = path.split(inpath)
	return actualDirname == expectedDirname  and  actualFilename == expectedFilename
end

test(compare_split('', '', ''), true)
test(compare_split('abc.txt', '', 'abc.txt'), true)
test(compare_split('/', '/', ''), true)
test(compare_split('/abc', '/', 'abc'), true)
test(compare_split('/abc/', '/abc/', ''), true)
test(compare_split('c:/', 'c:/', ''), true)
test(compare_split('c:/abc', 'c:/', 'abc'), true)
test(compare_split('c:/abc/', 'c:/abc/', ''), true)


test(path.trim('abc.txt'), 'abc.txt')
test(path.trim(' abc.txt'), 'abc.txt')
test(path.trim('abc.txt '), 'abc.txt')
test(path.trim('  \t  abc.txt \t \t \t '), 'abc.txt')


test(path.unescape(''), '')

if os.getenv("OS") == "Windows_NT" then
	test(path.unescape('"'), '')
	test(path.unescape('""'), '')
	test(path.unescape('" "'), ' ')
	test(path.unescape('"file with spaces'), 'file with spaces')
	test(path.unescape('"file with spaces"'), 'file with spaces')
else
	test(path.unescape('"'), '"')
	test(path.unescape('""'), '""')
	test(path.unescape('"\\ "'), '" "')
	test(path.unescape('"file\\ with\\ spaces'), '"file with spaces')
	test(path.unescape('"file\\ with\\ spaces"'), '"file with spaces"')
end
