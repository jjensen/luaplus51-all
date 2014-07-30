local ospath = require 'ospath'

scriptPath = ((debug.getinfo(1, "S").source:match("@(.+)[\\/]") or '.') .. '\\'):gsub('\\', '/'):lower()

function test(a, b)
	if a ~= b then
		print('Actual:', a)
		print('Expected:', b)
		assert()
	end
end

test(ospath.add_extension('', 'ext'), '.ext')
test(ospath.add_extension(' ', ' ext'), ' . ext')
test(ospath.add_extension('', '.ext'), '.ext')
test(ospath.add_extension('hello', 'ext'), 'hello.ext')
test(ospath.add_extension('hello', '.ext'), 'hello.ext')
test(ospath.add_extension('hello.txt', 'ext'), 'hello.txt.ext')
test(ospath.add_extension('hello.txt', '.ext'), 'hello.txt.ext')


test(ospath.add_slash(''), '/')
test(ospath.add_slash(' '), ' /')
test(ospath.add_slash('hello'), 'hello/')
test(ospath.add_slash(' hello'), ' hello/')
test(ospath.add_slash(' hello '), ' hello /')
test(ospath.add_slash('hello/'), 'hello/')


test(ospath.append('', 'filename.txt'), 'filename.txt')
test(ospath.append('', 'dir', 'filename.txt'), 'dir/filename.txt')
test(ospath.append('', 'dirA', '', 'dirB', 'filename.txt'), 'dirA/dirB/filename.txt')
test(ospath.append('..', 'filename.txt'), '../filename.txt')
test(ospath.append('root', 'filename.txt'), 'root/filename.txt')
test(ospath.append('root', 'dir', 'filename.txt'), 'root/dir/filename.txt')
test(ospath.append('root', 'dirA', '', 'dirB', 'filename.txt'), 'root/dirA/dirB/filename.txt')
test(ospath.append('root', 'dirA', '', 'dirB', '..', 'filename.txt'), 'root/dirA/dirB/../filename.txt')
test(ospath.append('root', 'dirA', '', '/dirB', '..', 'filename.txt'), '/dirB/../filename.txt')


test(ospath.join('', 'filename.txt'), 'filename.txt')
test(ospath.join('', 'dir', 'filename.txt'), 'dir/filename.txt')
test(ospath.join('', 'dirA', '', 'dirB', 'filename.txt'), 'dirA/dirB/filename.txt')
test(ospath.join('..', 'filename.txt'), '../filename.txt')
test(ospath.join('root', 'filename.txt'), 'root/filename.txt')
test(ospath.join('root', 'dir', 'filename.txt'), 'root/dir/filename.txt')
test(ospath.join('root', 'dirA', '', 'dirB', 'filename.txt'), 'root/dirA/dirB/filename.txt')
test(ospath.join('root', 'dirA', '', 'dirB', '..', 'filename.txt'), 'root/dirA/filename.txt')
test(ospath.join('c:/test/test2', '..', '.'), 'c:/test/')
test(ospath.join('c:/test/test2', '..', '.', 'filename.txt'), 'c:/test/filename.txt')
test(ospath.join('c:/test/test2', '..'), 'c:/test/')
test(ospath.join('c:/test/test2', '..', '..'), 'c:/')
test(ospath.join('c:/test/test2', '..', '..', '..'), 'c:/')


test(ospath.escape(''), '')
test(ospath.escape('filename.txt'), 'filename.txt')
if os.getenv("OS") == "Windows_NT" then
	test(ospath.escape(' '), '" "')
	test(ospath.escape('file name.txt'), '"file name.txt"')
else
	test(ospath.escape(' '), '\\ ')
	test(ospath.escape('file name.txt'), 'file\\ name.txt')
end


test(ospath.exists(scriptPath .. 'ospathtests.lua'), true)
test(ospath.exists(scriptPath .. 'ospathtests2.lua'), false)


test(ospath.find_extension(''), nil)
test(ospath.find_extension('filename'), nil)
test(ospath.find_extension('.lua'), 1)
test(ospath.find_extension('ospathtests.lua'), 12)
test(ospath.find_extension('ospathtests'), nil)


test(ospath.find_filename('ospathtests.lua'), 1)
test(ospath.find_filename('/ospathtests'), 2)
test(ospath.find_filename('c:/ospathtests'), 4)
test(ospath.find_filename('c:ospathtests'), 3)


test(ospath.get_extension(''), '')
test(ospath.get_extension('filename'), '')
test(ospath.get_extension('filename.ext'), '.ext')
test(ospath.get_extension('filename.txt.ext'), '.ext')


test(ospath.get_filename(''), '')
test(ospath.get_filename('filename'), 'filename')
test(ospath.get_filename('filename.ext'), 'filename.ext')
test(ospath.get_filename('c:/directory/filename.ext'), 'filename.ext')
test(ospath.get_filename('c:/directory/'), '')


test(ospath.is_directory(''), false)
test(ospath.is_directory(scriptPath .. 'ospathtests.lua'), false)
test(ospath.is_directory('.'), true)
test(ospath.is_directory('..'), true)
test(ospath.is_directory(scriptPath .. '../tests'), true)
test(ospath.is_directory(scriptPath .. '../tests/'), true)


test(ospath.is_file(''), nil)
test(ospath.is_file(scriptPath .. 'ospathtests.lua'), true)
test(ospath.is_file('.'), false)
test(ospath.is_file('..'), false)
test(ospath.is_file(scriptPath .. '../tests'), false)
test(ospath.is_file(scriptPath .. '../tests/'), nil)


test(ospath.is_relative(''), true)
test(ospath.is_relative('filename.ext'), true)
test(ospath.is_relative('/filename.ext'), false)
test(ospath.is_relative('c:/filename.ext'), false)


test(ospath.is_root(''), false)
test(ospath.is_root('filename.ext'), false)
test(ospath.is_root('/filename.ext'), true)
test(ospath.is_root('c:/filename.ext'), true)


test(ospath.is_unc(''), false)
test(ospath.is_unc('filename.ext'), false)
test(ospath.is_unc('/filename.ext'), false)
test(ospath.is_unc('c:/filename.ext'), false)
test(ospath.is_unc('\\\\share'), true)
test(ospath.is_unc('//share'), true)


local cwd = ospath.getcwd():gsub('\\', '/')
test(ospath.make_absolute(''), cwd)
test(ospath.make_absolute('.'), cwd .. '/')
test(ospath.make_absolute('..'), cwd:match('(.+)/') .. '/')
test(ospath.make_absolute('abc'), cwd .. '/abc')


test(ospath.make_backslash(''), '')
test(ospath.make_backslash(' '), ' ')
test(ospath.make_backslash('\\\\abc'), '\\\\abc')
test(ospath.make_backslash('//abc'), '\\\\abc')
test(ospath.make_backslash('c:/abc/def/'), 'c:\\abc\\def\\')


test(ospath.make_slash(''), '')
test(ospath.make_slash(' '), ' ')
test(ospath.make_slash('\\\\abc'), '//abc')
test(ospath.make_slash('//abc'), '//abc')
test(ospath.make_slash('c:\\abc\\def\\'), 'c:/abc/def/')


do
	os.remove('out.dat')

	local file = io.open('out.dat', 'wb')
	file:write('junk')
	file:close()

	test(ospath.is_writable('out.dat'), true)
	if os.getenv('OS') == 'Windows_NT' then
		ospath.chmod('out.dat', 'r')
	else
		ospath.chmod('out.dat', 0444)
	end
	test(ospath.is_writable('out.dat'), false)
	ospath.make_writable('out.dat')
	test(ospath.is_writable('out.dat'), true)

	os.remove('out.dat')
end	


test(ospath.match('', ''), true)
test(ospath.match('', '*'), true)
test(ospath.match('', '*.*'), false)
test(ospath.match('', 'a*'), false)
test(ospath.match('abcdefg.txt', 'a*'), true)
test(ospath.match('abcdefg.txt', 'a*b*c?e*'), true)
test(ospath.match('abcdefg.txt', 'a*b*c?f*'), false)
test(ospath.match('abcdefg.txt', '*.'), false)
test(ospath.match('abcdefg.txt', '*.t'), false)
test(ospath.match('abcdefg.txt', '*.t*'), true)
test(ospath.match('abcdefg.txt', '*.t'), false)
test(ospath.match('abcdefg.txt', '*.*t'), true)
test(ospath.match('abcdefg.txt', '*.*x'), false)
test(ospath.match('abcdefg.txt', '*.txt'), true)


test(ospath.remove_directory(''), '')
test(ospath.remove_directory(' \t'), ' \t')
test(ospath.remove_directory('abc'), 'abc')
test(ospath.remove_directory('/abc.'), 'abc.')
test(ospath.remove_directory('/dir/abc.'), 'abc.')
test(ospath.remove_directory('c:/abc.'), 'abc.')
test(ospath.remove_directory('c:/dir/abc'), 'abc')


test(ospath.remove_extension(''), '')
test(ospath.remove_extension(' \t'), ' \t')
test(ospath.remove_extension('abc'), 'abc')
test(ospath.remove_extension('abc.'), 'abc')
test(ospath.remove_extension('abc.txt'), 'abc')
test(ospath.remove_extension('abc.txt.dat'), 'abc.txt')


test(ospath.remove_filename(''), '')
test(ospath.remove_filename(' \t'), '')
test(ospath.remove_filename('abc'), '')
test(ospath.remove_filename('/abc.'), '/')
test(ospath.remove_filename('/dir/abc.'), '/dir/')
test(ospath.remove_filename('c:/abc.'), 'c:/')
test(ospath.remove_filename('c:/dir/abc'), 'c:/dir/')


test(ospath.remove_slash(''), '')
test(ospath.remove_slash(' \t'), ' \t')
test(ospath.remove_slash('abc'), 'abc')
test(ospath.remove_slash('abc/'), 'abc')


test(ospath.replace_extension('', 'ext'), '.ext')
test(ospath.replace_extension('', '.ext'), '.ext')
test(ospath.replace_extension('hello', 'ext'), 'hello.ext')
test(ospath.replace_extension('hello', '.ext'), 'hello.ext')
test(ospath.replace_extension('hello.txt', 'ext'), 'hello.ext')
test(ospath.replace_extension('hello.txt', '.ext'), 'hello.ext')
test(ospath.replace_extension('hello.txt.dat', 'ext'), 'hello.txt.ext')
test(ospath.replace_extension('hello.txt.dat', '.ext'), 'hello.txt.ext')


test(ospath.simplify(''), '')
test(ospath.simplify('abc'), 'abc')
test(ospath.simplify('.abc'), '.abc')
test(ospath.simplify('./abc'), 'abc')
test(ospath.simplify('..abc'), '..abc')
test(ospath.simplify('../abc'), '../abc')
test(ospath.simplify('abc/////def'), 'abc/def')
test(ospath.simplify('abc/././././def'), 'abc/def')
test(ospath.simplify('c:/somedir/.././././def'), 'c:/def')
test(ospath.simplify('abc/.././././def'), 'def')
test(ospath.simplify('abc/ABC/../../../../def'), 'def')
test(ospath.simplify('c:\\abc\\ABC\\../..\\../..\\def'), 'c:/def')
test(ospath.simplify('\\\\uncserver\\pathA\\file.txt'), '\\\\uncserver/pathA/file.txt')


function compare_split(inpath, expectedDirname, expectedFilename)
	local actualDirname, actualFilename = ospath.split(inpath)
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


test(ospath.trim('abc.txt'), 'abc.txt')
test(ospath.trim(' abc.txt'), 'abc.txt')
test(ospath.trim('abc.txt '), 'abc.txt')
test(ospath.trim('  \t  abc.txt \t \t \t '), 'abc.txt')


test(ospath.unescape(''), '')

if os.getenv("OS") == "Windows_NT" then
	test(ospath.unescape('"'), '')
	test(ospath.unescape('""'), '')
	test(ospath.unescape('" "'), ' ')
	test(ospath.unescape('"file with spaces'), 'file with spaces')
	test(ospath.unescape('"file with spaces"'), 'file with spaces')
else
	test(ospath.unescape('"'), '"')
	test(ospath.unescape('""'), '""')
	test(ospath.unescape('"\\ "'), '" "')
	test(ospath.unescape('"file\\ with\\ spaces'), '"file with spaces')
	test(ospath.unescape('"file\\ with\\ spaces"'), '"file with spaces"')
end

local cwd = ospath.getcwd()
test(ospath.mkdir("Foo.test/"), true)
test(ospath.chdir("Foo.test"), true)
test(ospath.join(ospath.getcwd()), ospath.join(cwd, 'Foo.test'))

test(ospath.chdir(".."), true)
test(ospath.getcwd(), cwd)
test(ospath.remove("Foo.test/"), true)
test(ospath.chdir("Foo.test"), nil)


