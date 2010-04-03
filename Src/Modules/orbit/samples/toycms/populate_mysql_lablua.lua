local db = 'lablua'

require "luasql.mysql"
require "orbit.model"

local env = luasql.mysql()
local conn = env:connect(db, "root", "password")

local mapper = orbit.model.new("toycms_", conn, "mysql")



-- Table post

local t = mapper:new('post')

-- Record 1

local rec = {
 ["id"] = 1,
 ["external_url"] = "",
 ["body"] = "		  <h3>About</h3>\
		  \
		  <p>Lablua is a research lab at <a href=\"http://www.puc-rio.br\">PUC-Rio</a>, affiliated with its\
		  <a href=\"http://www.inf.puc-rio.br\">Computer Science Department</a>. It is dedicated to research\
		  about programming languages, with emphasis on research involving the <a href=\"http://www.lua.org\">Lua</a>\
		  language. Lablua was founded on May 2004 by <a href=\"http://www.inf.puc-rio.br/~roberto\">Prof. Roberto Ierusalimschy</a>.\
		  Since then its members have produced one PhD thesis and two MSc dissertations.</p>\
\
		  <p>Lua is a scripting language not totally unlike Tcl, Perl, or Python. Like Tcl, Lua is an \"embedded language\", in the sense that embedding the interpreter into your program is a trivial task, and it is very easy to interface Lua with other languages like C, C++, or even Fortran. Like Python, Lua has a clear and intuitive syntax. Like all those three, Lua is an interpreted language with dynamic typing, and with several reflexive facilities.</p>\
		  \
		  <p>In these pages you can find information about current and past projects, publications and members. Please\
		  follow the links above to go to the other sections of the web site.</p>\
\
		  <h3>Contact</h3>\
		  \
		  <p>Lablua is situated at PUC-Rio's campus, near the Cardeal Leme building. Prof. Ierusalimschy's contact\
		  information is in his <a href=\"http://www.inf.puc-rio.br/~roberto\">web page</a>. You can also contact\
		  Lablua by phone: 55-21-3527-1497 Ext. 4523.</p>",
 ["published_at"] = 1183576020,
 ["image"] = "",
 ["title"] = "Home Page",
 ["comment_status"] = "closed",
 ["section_id"] = 1,
 ["published"] = true,
 ["abstract"] = "",
 ["user_id"] = 1}
rec = t:new(rec)
rec:save(true)

-- Record 2

local rec = {
 ["id"] = 2,
 ["external_url"] = "http://www.lua.org",
 ["body"] = "",
 ["published_at"] = 1183576020,
 ["image"] = "",
 ["title"] = "Lua.org",
 ["comment_status"] = "closed",
 ["section_id"] = 5,
 ["published"] = true,
 ["abstract"] = "",
 ["user_id"] = 1}
rec = t:new(rec)
rec:save(true)

-- Record 3

local rec = {
 ["id"] = 3,
 ["external_url"] = "http://luaforge.net/",
 ["body"] = "",
 ["published_at"] = 1183489980,
 ["image"] = "",
 ["title"] = "Luaforge",
 ["comment_status"] = "closed",
 ["section_id"] = 5,
 ["published"] = true,
 ["abstract"] = "",
 ["user_id"] = 1}
rec = t:new(rec)
rec:save(true)

-- Record 4

local rec = {
 ["id"] = 4,
 ["external_url"] = "http://www.lua.inf.puc-rio.br/luaclr",
 ["body"] = "",
 ["published_at"] = 1180984800,
 ["image"] = "",
 ["title"] = "LuaCLR",
 ["comment_status"] = "closed",
 ["section_id"] = 2,
 ["published"] = true,
 ["abstract"] = "<p><strong>LuaCLR</strong> is a newer implementation of\
	          Lua on the CLR that targets Lua 5.1 and compiles from \
	          source (without using the Lua parser and lexer).</p>",
 ["user_id"] = 1}
rec = t:new(rec)
rec:save(true)

-- Record 5

local rec = {
 ["id"] = 5,
 ["external_url"] = "",
 ["body"] = "			<h2>libscript</h2>\
\
<p>\
<strong>libscript</strong> is a plugin-based library designed to add\
language-independent extensibility to applications.\
</p>\
\
<p>\
It allows to decouple an application from the virtual machines provided by the\
various scripting languages. The main library, <em>libscript</em>, is a thin layer that provides a\
language-independent scripting API, allowing the application to register its\
functions and invoke code to be performed. Libscript then invokes the\
appropriate plugin (<em>libscript-python</em>, <em>libscript-ruby</em>, <em>libscript-lua</em>, etc.)\
to run the code. This way, the application can support all those scripting\
languages without adding each of them as a dependency.\
</p>\
\
<p>\
<strong>libscript</strong> was developed by <a\
href=\"http://www.inf.puc-rio.br/~hisham\">Hisham Muhammad</a> as a case study for\
his MSc dissertation at <a href=\"http://www.puc-rio.br/\">PUC-RIO</a>.\
</p> \
\
<p>For more information, please go the the <a href=\"http://libscript.sourceforge.net\">Sourceforge page</a>.</p>\
\
",
 ["published_at"] = 1183576860,
 ["image"] = "",
 ["title"] = "libscript",
 ["comment_status"] = "closed",
 ["section_id"] = 2,
 ["published"] = true,
 ["abstract"] = "<p><strong>libscript</strong> is a plugin-based library designed to add language-independent\
		  extensibility to applications.</p>",
 ["user_id"] = 1}
rec = t:new(rec)
rec:save(true)

-- Record 6

local rec = {
 ["id"] = 6,
 ["external_url"] = "",
 ["body"] = "			<h2>LuaDec, Lua bytecode decompiler</h2>\
\
		        <h3>Overview</h3>\
\
		  <p>LuaDec is a decompiler for the Lua language. It takes compiled Lua bytecodes and attempts to produce equivalent\
		   Lua source code on standard output. It targets Lua 5.0.2.<p>\
\
\
<p>\
The decompiler performs two passes. The first pass is considered a \"preliminary\
pass\". Only two actions are performed: addresses of backward jumps are gathered\
for <code>while</code> constructs, and variables referenced by <code>CLOSE</code>\
\
instructions are taken note of in order to identify the point where explicit\
<code>do</code> blocks should be opened.\
</p>\
<p>\
The actual decompilation takes place in the second, \"main pass\". In this pass,\
the instructions are symbolically interpreted. As functions are traversed\
recursively, the symbolic content of registers are kept track of, as well as\
a few additional data structures: two register stacks, one for \"variable\
registers\" (registers currently allocated for variables) one for \"temporary\
registers\" (registers holding temporary values); and a list of boolean conditions. \
</p>\
<p>\
Instructions like <code>ADD</code> and <code>MOVE</code> combine the string\
representation of symbols and move them around in registers. Emission of actual\
statements is delayed until the temporaries stack is empty, so that constructs\
like <code>a,b=b,a</code> are processed correctly. \
\
</p>\
<p>\
The list of boolean conditions accumulate pairs of relational operators (or\
<code>TEST</code>s and jumps). The translation of a list into a boolean expression\
is done in three stages. In the first stage, jump addresses are checked to identify\
\"then\" and \"else\" addresses and to break the list into smaller lists in the case of\
nested <code>if</code> statements. In the second stage, the relations between the\
jumps is analyzed serially to devise a parenthization scheme. In the third scheme,\
taking into account the parenthization and an 'inversion' flag (used to tell\
\"jump-if-true\" from \"jump-if-false\" expressions), the expression is printed,\
recursively.\
</p>\
<p>\
Two forms of \"backpatching\" are used in the main processing pass: boolean conditions\
for <code>while</code> constructs are inserted in the marked addresses (as noted in\
the first pass), and <code>do</code> blocks are added as necessary, according to the\
liveness information of local variables.\
\
</p>\
		  <p>LuaDec is written by <a href=\"http://www.inf.puc-rio.br/~hisham\">Hisham Muhammad</a>, and is licensed\
		  under the same terms as Lua. Windows binaries contributed by Fabio Mascarenhas.</a></p>\
\
                       <h3>Status</h3>\
\
<p>\
LuaDec, in its current form, is <i>not</i> a complete decompiler. It does\
succeed in decompiling most of the Lua constructs, so it could be used as\
a tool to aid in retrieving lost sources.\
</p>\
\
<p>\
The situations where LuaDec \"get it wrong\" are usually related to deciding\
whether a sequence of relational constructs including <code>TEST</code> operators\
are part of an assignment or an <code>if</code> construct. Also, the \"single pass\"\
nature of the symbolic interpreter fails at some corner cases where there simply\
is not enough information in the sequence of operator/jump pairs to identify what\
are the \"then\" and \"else\" addresses. This is an example of such a case:\
</p>\
\
<pre>\
1       [2]     LOADNIL         0 2 0\
2       [3]     JMP             0 16    ; to 19<font color=\"#0000c0\"><b>\
3       [4]     EQ              0 1 250 ; - 2\
4       [4]     JMP             0 2     ; to 7\
5       [4]     TEST            2 2 1\
6       [4]     JMP             0 5     ; to 12\
7       [4]     EQ              0 1 251 ; - 3\
8       [4]     JMP             0 3     ; to 12\
9       [4]     LOADK           3 2     ; 1\
10      [4]     TEST            3 3 1\
11      [4]     JMP             0 0     ; to 12</b></font>\
12      [6]     LOADK           0 2     ; 1\
13      [7]     JMP             0 7     ; to 21\
14      [8]     LOADK           0 0     ; 2\
15      [8]     JMP             0 3     ; to 19\
16      [10]    LOADK           0 1     ; 3\
17      [11]    JMP             0 3     ; to 21\
18      [12]    LOADK           0 4     ; 4\
19      [3]     TEST            1 1 1\
20      [3]     JMP             0 -18   ; to 3\
21      [14]    RETURN          0 1 0\
</pre>\
\
<pre>\
local a, x, y\
while x do\
   if <font color=\"#0000c0\"><b>((x==2) and y) or ((x==3) and 1) or 0</b></font>\
\
   then\
      a = 1\
   do break end\
      a = 2\
   else\
      a = 3\
   do break end\
      a = 4\
   end\
   a = 5\
end\
</pre>\
\
<p>\
Notice that there is no reference to the \"else\" address in the highlighted sequence.\
Only with a more sophisticated block analysis it would be possible to identify the\
varying purposes of the <code>JMP</code> instructions at addresses 13, 15 and 17.\
</p>\
\
<p>\
For illustrational purposes, here are the results of running LuaDec on the\
bytecodes generated by the Lua demos included in the <code>test/</code> subdirectory\
of the official Lua distribution, as of version 0.4.\
</p>\
\
<table>\
<tr><td><b>bisect.lua</b> - <font color=\"#006000\">works</font></td></tr>\
<tr><td><b>cf.lua</b> - <font color=\"#006000\">works</font></td></tr>\
<tr><td><b>echo.lua</b> - <font color=\"#006000\">works</font></td></tr>\
\
<tr><td><b>env.lua</b> - <font color=\"#006000\">works</font></td></tr>\
<tr><td><b>factorial.lua</b> - <font color=\"#006000\">works</font></td></tr>\
<tr><td><b>fibfor.lua</b> - <font color=\"#006000\">works</font></td></tr>\
<tr><td><b>fib.lua</b> - <font color=\"#006000\">works</font></td></tr>\
\
<tr><td><b>globals.lua</b> - <font color=\"#006000\">works</font></td></tr>\
<tr><td><b>hello.lua</b> - <font color=\"#006000\">works</font></td></tr>\
<tr><td><b>life.lua</b> - <font color=\"#006000\">works</font></td></tr>\
<tr><td><b>luac.lua</b> - <font color=\"#006000\">works</font></td></tr>\
\
<tr><td><b>printf.lua</b> - <font color=\"#006000\">works</font></td></tr>\
<tr><td><b>readonly.lua</b> - <font color=\"#006000\">works</font></td></tr>\
<tr><td><b>sieve.lua</b> - <font color=\"#006000\">works</font></td></tr>\
<tr><td><b>sort.lua</b> - <font color=\"#006000\">works</font></td></tr>\
\
<tr><td><b>table.lua</b> - <font color=\"#006000\">works</font></td></tr>\
<tr><td><b>trace-calls.lua</b> - <strike><font color=\"#ff0000\">fails assertion</font></strike> <font color=\"#006000\">works</font> (fixed in 0.4)</td></tr>\
<tr><td><b>trace-globals.lua</b> - <font color=\"#006000\">works</font></td></tr>\
\
<tr><td><b>undefined.lua</b> - <font color=\"#006000\">works</font></td></tr>\
<tr><td><b>xd.lua</b> - <font color=\"#006000\">works</font></td></tr>\
</table>\
\
<p></p>\
\
<h3>Running it</h3>\
\
<p>\
To try out LuaDec:\
</p>\
\
<pre>\
make\
bin/luac test/life.lua\
bin/luadec luac.out > newlife.lua\
bin/lua newlife.lua\
</pre>\
\
<p>\
LuaDec includes a <code><b>-d</b></code> flag, which displays the progress\
of the symbolic interpretation: for each bytecode, the variable stacks\
and the code generated so far are shown.\
</p>\
\
<h3>Operation modes</h3>\
\
<p>\
In its default operation mode, LuaDec recursively processes the program functions,\
starting from the outmost chunk (\"main\"). When LuaDec detects a decompilation error\
(by doing sanity checks on its internal data structures), Luadec outputs the portion\
of the function decompiled so far, and resumes decompilation in the outer closure.\
</p>\
\
<p>\
There is an alternative operation mode, set with the <code><b>-f</b></code> flag,\
where all functions are decompiled separately. This does not generate a .lua file\
structured like the original program, but it is useful in the cases where a\
decompilation error makes a closure declaration \"unreachable\" in the default\
operation mode. This allows a larger portion of the sources to be restored in\
the event of errors.\
</p>\
\
<h3>Download and Feedback</h3>\
\
<p>To download LuaDec, and send any comments, questions or bug reports, please go to the <a href=\"http://luaforge.net/projects/luadec\">LuaForge page</a>.</p>\
\
",
 ["published_at"] = 1183576920,
 ["image"] = "",
 ["title"] = "LuaDec",
 ["comment_status"] = "closed",
 ["section_id"] = 2,
 ["published"] = true,
 ["abstract"] = "<p><strong>LuaDec</strong> decompiles Lua bytecodes, reconstructing the original source code (or an approximation\
		  of it.</p>",
 ["user_id"] = 1}
rec = t:new(rec)
rec:save(true)

-- Record 7

local rec = {
 ["id"] = 7,
 ["external_url"] = "",
 ["body"] = "			<h2>Lua2IL, a Lua to CIL compiler</h2>\
\
		        <h3>Overview</h3>\
		  \
			<p>Lua2IL compiles scripts written in the <a href=\"http://www.lua.org\">Lua</a> language to\
			Common Language Infrastructure (CLI) assemblies. The compiler generates pure managed Common\
			Intermediate Language (CIL) code. Besides compiling scripts to CIL, Lua2IL also lets them\
			interface with CLI objects written in other languages.</p>\
					\
			<p>For an interpreted language, Lua2IL generates efficient code, with the code being\
			about three times faster than similar JScript code compiled by the Microsoft JScript.NET\
			compiler bundled with .NET Framework 1.1 (without type optimizations).</p>\
\
		        <p>A paper about the Lua2IL compiler was published in a special edition of the\
		        <a href=\"http://www.jucs.org\">Journal of Universal Computer Science</a>. You can read the paper\
		        <a href=\"lua2il_jucs.pdf\">here</a>.</p>\
\
		        <p>Lua2IL is designed and implemented by Fabio Mascarenhas.</p>\
\
		        <h3>Download</h3>\
		  \
			<p>The compiler is a prototype. It compiles the full Lua 5.0 language, but doesn't include\
			most of the Lua standard library. You can download Lua2IL <a href=\"lua2il-0.5.zip\">here</a>.</p>\
\
		        <h3>Feedback</h3>\
\
		        <p>Please send questions or comments to <a href=\"mailto:mascarenhas@acm.org\">Fabio Mascarenhas</a>.</p>\
\
",
 ["published_at"] = 1183576980,
 ["image"] = "",
 ["title"] = "Lua2IL",
 ["comment_status"] = "closed",
 ["section_id"] = 2,
 ["published"] = true,
 ["abstract"] = "<p><strong>Lua2IL</strong> is a prototype of an implementation of the Lua language on the CLR. It converts\
		  Lua bytecodes to the CLR intermediate language, so Lua scripts can run on the CLR without\
		  the Lua interpreter.</p>",
 ["user_id"] = 1}
rec = t:new(rec)
rec:save(true)

-- Record 8

local rec = {
 ["id"] = 8,
 ["external_url"] = "",
 ["body"] = "			<h2>LuaInterface: Scripting CLR objects with Lua</h2>\
		  \
			<h3>Overview</h3>\
\
		  <p>LuaInterface is a library for brdging the\
		  <a href=\"http://www.lua.org\">Lua</a> language and Microsoft .NET platform's\
		  <a href=\"http://msdn.microsoft.com/net/ecma/\">Common Language Runtime \
		  (CLR)</a>. LuaInterface is a full consumer of the Common Language Specification (CLS),\
		  so Lua scripts can use LuaInterface to instantiate CLR objects, access their properties,\
		  call their methods, and even handle their events with Lua functions. Any CLR program\
		  can also use LuaInterface to run Lua scripts and modify the scripts' environment. This is\
		  a short, working example of LuaInterface in action (it shows a window, with two buttons,\
		  on the screen):</p>\
\
<pre>\
    require(\"luanet\")\
    \
    Form = luanet.System.Windows.Forms.Form\
    Button = luanet.System.Windows.Forms.Button\
    Point = luanet.System.Drawing.Point\
    \
    mainForm = Form()\
    buttonOk = Button()\
    buttonCancel = Button()\
    \
    buttonOk.Text = \"Ok\"\
    buttonCancel.Text = \"Cancel\"\
    buttonOk.Location = Point(10,10)\
    buttonCancel.Location = Point(buttonOk.Left, buttonOk.Height +\
      buttonOk.Top + 10)\
    mainForm.Controls:Add(buttonOk)\
    mainForm.Controls:Add(buttonCancel)\
    mainForm.StartPosition = \
      luanet.System.Windows.Forms.FormStartPosition.CenterScreen\
    \
    function handleMouseUp(sender,args)\
      print(sender:ToString() .. \" MouseUp!\")\
    end\
    \
    handlerUp = buttonOk.MouseUp:Add(handleMouseUp)\
    handlerClick = buttonCancel.Click:Add(os.exit)\
    \
    mainForm:ShowDialog()\
</pre>\
\
		  <p>You can find more about LuaInterface by reading this <a href=\"luainterface.pdf\">paper</a>,\
		  published in the proceedings of the 8th Brazilian Symposium on Programming Languages, or\
		  download the library and try it out.</p>\
\
		  <p><a href=\"mailto:mascarenhas@acm.org\">Fabio Mascarenhas</a> did the initial design and implementation of LuaInterface, and it is now being actively maintained by <a href=\"http://luaforge.net/users/kevinh/\">Kevin Hester</a>.</p>\
		  \
		  <h3>Download</h3>\
\
		  <p>LuaInterface is free software (MIT license), and can be downloaded from its <a href=\"http://luaforge.net/projects/luainterface\">LuaForge page</a>. There are versions for Lua 5.1 and 5.0, and for use in versions 1.1 and 2.0 of the CLR.</p>\
		  \
\
            <h3>Feedback</h3>\
            \
            <p>Please send comments, suggestions or bug reports through the <a href=\"http://luaforge.net/projects/luainterface\">LuaForge page</a>.</p>\
\
",
 ["published_at"] = 1183577040,
 ["image"] = "",
 ["title"] = "LuaInterface",
 ["comment_status"] = "closed",
 ["section_id"] = 2,
 ["published"] = true,
 ["abstract"] = "<p><strong>LuaInterface</strong> is a bridge between Lua and the Common Language Runtime. It lets Lua\
		  scripts instantiate and use CLR objects, and makes it easier for CLR programs to embed\
		  the Lua Interpreter.</p>",
 ["user_id"] = 1}
rec = t:new(rec)
rec:save(true)

-- Record 9

local rec = {
 ["id"] = 9,
 ["external_url"] = "",
 ["body"] = "		  <h2>Lua.NET: Integrating Lua with the CLI</h2>\
		  \
<p><a href=\"http://www.lua.org\">Lua</a> is a scripting language not totally unlike \
						Tcl, Perl, or Python. Like Tcl, Lua is an \"embedded language\", in the sense that embedding \
						the interpreter into your\
						program is a trivial task, and it is very easy to interface Lua with other languages\
						like C, C++, or even Fortran. Like Python, Lua has a clear and intuitive syntax. \
						Like all those three, Lua is an interpreted language with dynamic typing, and with \
						several reflexive facilities.</p>\
\
            \
            <p>What sets Lua apart from those languages is its portability, simplicity, and small size. \
						Lua is written in ANSI C, and runs without modifications in almost any platform (MS-DOS, \
						all versions of Windows, all flavors of Unix, plus X-Box, PlayStation II, OS/2, BeOS, \
						EPOC, etc.). The whole program lua.exe has less than 200 Kbytes. Its simplicity led other \
						groups to adopt Lua as a scripting language for other scripting languages (see, for \
						instance, <a href=\"http://ruby-lua.unolotiene.com\">Ruby-Lua</a>).</p>\
            \
            <p>Currently, Lua has a strong presence whenever programmers need a light, efficient,\
						and portable language. It is being used by some tens of thousands programmers around \
						the world, both in research and in industrial projects. Lua has been successfully \
						used in games (e.g. Grim Fandango, Escape from Monkey Island, MK2, Baldur's Gate), \
						in robots (e.g. Crazy Ivan, that won the Danish RoboCup in 2000 and 2001), and \
						several other applications (e.g. a hot-swappable Ethernet switch (CPC4400), a genetic \
						sequence visualization system (GUPPY), \"The most Linux on one floppy disk\" (tomsrtbt)). \
						An extended list of applications using Lua can be found  \
						<a href=\"http://www.lua.org/uses.html\">here</a>.</p>\
            \
            <p>The Lua.NET project integrates Lua with the \
						<a href=\"http://msdn.microsoft.com/netframework/programming/clr/\">Common Language Infrastructure</a>,\
						a framework for language interoperability. \
						This integration allows Lua \
						to act both as a \"client\" language and as a \"server\" language, although with a\
						limited capacity for the latter. As a client, Lua scripts can access components available \
						through the CLI. As a server, Lua scripts can implement new components accessible by \
						other languages integrated with the CLI.<p>\
            \
            <p>Because Lua is an interpreted language with dynamic typing, its integration with \
						the CLI demands a dynamic nature. Lua.NET employs two approaches for this integration. \
						The first uses the same techniques used to implement <a href=\"http://www.tecgraf.puc-rio.br/luaorb\">LuaOrb</a>, \
						a scripting tool that can	access and implement CORBA, COM and Java \
						components. The approach is implemented by the <a href=\"luainterface/\">LuaInterface</a> library.</p>\
\
            \
            <p>The second approach compiles Lua to the CLI's Common Intermediate \
						Language, instead of its own bytecode representation. This approach is implemented\
						by the <a href=\"lua2il/\">Lua2IL</a> compiler.</p>\
\
						<p>The authors of the Lua.NET project are <a href=\"http://www.inf.puc-rio.br/~roberto/\">Roberto Ierusalimschy</a>,\
						<a href=\"http://www.inf.puc-rio.br/~rcerq/\">Renato Cequeira</a> and <a href=\"mailto:mascarenhas@acm.org\">Fabio Mascarenhas</a>.\
						The project is sponsored by <a href=http://research.microsoft.com/programs/europe/rotor/\">Microsoft Research</a>\
\
						and <a href=\"http://www.capes.gov.br\">CAPES.</a></p>\
          \
	  <h3>Publications</h3>\
\
	  <p>Fabio Mascarenhas, Roberto Ierusalimschy. <a href=\"http://www.jucs.org/jucs_10_7/luainterface_scripting_the_net\">LuaInterface: Scripting the .NET CLR with Lua</a>. Journal of Universal Computer Science, 10(7):892-908, July 2004.</p>\
	  \
          <p>Fabio Mascarenhas, Roberto Ierusalimschy. <a href=\"lua2il_jucs.pdf\">Running Lua Scripts on the CLR Through Bytecode Translation</a>. Due for publication in a special edition of the Journal of Universal Computer Science.</p>\
\
",
 ["published_at"] = 1183663440,
 ["image"] = "",
 ["title"] = "Lua.NET",
 ["comment_status"] = "closed",
 ["section_id"] = 2,
 ["published"] = true,
 ["abstract"] = "<p><strong>Lua.NET</strong> was a project sponsored by <a href=\"http://research.microsoft.com\">Microsoft Research</a>\
		  to integrate Lua with the Common Language Runtime. It generated both <strong>LuaInterface</strong> and\
		  <strong>Lua2IL</strong>.</p>",
 ["user_id"] = 1}
rec = t:new(rec)
rec:save(true)

-- Record 10

local rec = {
 ["id"] = 10,
 ["external_url"] = "",
 ["body"] = "<p><a href=\"http://www.jucs.org/jucs_11_7/running_lua_scripts_on\">Running Lua scripts on the CLR through bytecode translation</a>, by Fabio Mascarenhas and Roberto Ierusalimschy. Journal of Universal Computer Science 11 #7 (2005) 1275-1290.</p>",
 ["published_at"] = 1183577760,
 ["image"] = "",
 ["title"] = "Running Lua scripts on the CLR through bytecode translation",
 ["comment_status"] = "closed",
 ["section_id"] = 6,
 ["published"] = true,
 ["abstract"] = "",
 ["user_id"] = 1}
rec = t:new(rec)
rec:save(true)

-- Record 11

local rec = {
 ["id"] = 11,
 ["external_url"] = "",
 ["body"] = "<p><a href=\"http://www.jucs.org/jucs_10_7/coroutines_in_lua\">Coroutines in Lua</a>, by Ana Lúcia de Moura, Noemi Rodriguez, and Roberto Ierusalimschy. Journal of Universal Computer Science 10 #7 (2004) 910-925.</p>",
 ["published_at"] = 1183491420,
 ["image"] = "",
 ["title"] = "Coroutines in Lua",
 ["comment_status"] = "closed",
 ["section_id"] = 6,
 ["published"] = true,
 ["abstract"] = "",
 ["user_id"] = 1}
rec = t:new(rec)
rec:save(true)

-- Record 12

local rec = {
 ["id"] = 12,
 ["external_url"] = "",
 ["body"] = "<p><a href=\"http://www.jucs.org/jucs_10_7/luainterface_scripting_the_net\">LuaInterface: Scripting .NET CLR with Lua</a>, by Fabio Mascarenhas and Roberto Ierusalimschy. Journal of Universal Computer Science 10 #7 (2004) 892-909.</p>",
 ["published_at"] = 1183405020,
 ["image"] = "",
 ["title"] = "LuaInterface: Scripting .NET CLR with Lua",
 ["comment_status"] = "closed",
 ["section_id"] = 6,
 ["published"] = true,
 ["abstract"] = "",
 ["user_id"] = 1}
rec = t:new(rec)
rec:save(true)

-- Record 13

local rec = {
 ["id"] = 13,
 ["external_url"] = "",
 ["body"] = "<p><a href=\"http://www.tecgraf.puc-rio.br/~moura/publications/thesis.pdf.gz\">Revisitando co-rotinas</a>, by Ana Lúcia de Moura. Ph.D. thesis, Department of Computer Science, PUC-Rio, Sep 2004.</p>",
 ["published_at"] = 1183577880,
 ["image"] = "",
 ["title"] = "Revisitando co-rotinas",
 ["comment_status"] = "closed",
 ["section_id"] = 7,
 ["published"] = true,
 ["abstract"] = "",
 ["user_id"] = 1}
rec = t:new(rec)
rec:save(true)

-- Record 14

local rec = {
 ["id"] = 14,
 ["external_url"] = "",
 ["body"] = "<p>Estudo sobre APIs de Linguagens de Script, by Hisham Muhammad. M.Sc. dissertation, Department of Computer Science, PUC-Rio, Aug 2006.</p>",
 ["published_at"] = 1183491480,
 ["image"] = "",
 ["title"] = "Estudo sobre APIs de Linguagens de Script",
 ["comment_status"] = "closed",
 ["section_id"] = 7,
 ["published"] = true,
 ["abstract"] = "",
 ["user_id"] = 1}
rec = t:new(rec)
rec:save(true)

-- Record 15

local rec = {
 ["id"] = 15,
 ["external_url"] = "",
 ["body"] = "<p><a href=\"mascarenhas04integracao.pdf\">Integração entre a linguagem Lua e o Common Language Runtime</a>, by Fabio Mascarenhas. M.Sc. dissertation, Department of Computer Science, PUC-Rio, Mar 2004.</a>",
 ["published_at"] = 1183405140,
 ["image"] = "",
 ["title"] = "Integração entre a linguagem Lua e o Common Language Runtime",
 ["comment_status"] = "closed",
 ["section_id"] = 7,
 ["published"] = true,
 ["abstract"] = "",
 ["user_id"] = 1}
rec = t:new(rec)
rec:save(true)

-- Record 16

local rec = {
 ["id"] = 16,
 ["external_url"] = "",
 ["body"] = "<p><a href=\"hisham06study.pdf\">A study on scripting language APIs</a>, by Hisham Muhammad. M.Sc. dissertation (english translation), Department of Computer Science, PUC-Rio, Aug 2006.</p>",
 ["published_at"] = 1183577940,
 ["image"] = "",
 ["title"] = "A study on scripting language APIs",
 ["comment_status"] = "closed",
 ["section_id"] = 8,
 ["published"] = true,
 ["abstract"] = "",
 ["user_id"] = 1}
rec = t:new(rec)
rec:save(true)

-- Record 17

local rec = {
 ["id"] = 17,
 ["external_url"] = "",
 ["body"] = "<p><a href=\"http://www.inf.puc-rio.br/~roberto\">Roberto Ierusalimschy</a>, Associate Professor, Computer Science\
		  Department, PUC-Rio.</p>",
 ["published_at"] = 1183578360,
 ["image"] = "",
 ["title"] = "Roberto Ierusalimschy",
 ["comment_status"] = "closed",
 ["section_id"] = 9,
 ["published"] = true,
 ["abstract"] = "",
 ["user_id"] = 1}
rec = t:new(rec)
rec:save(true)

-- Record 18

local rec = {
 ["id"] = 18,
 ["external_url"] = "",
 ["body"] = "<p><a href=\"mailto:mascarenhas@acm.org\">Fabio Mascarenhas</a>, PhD Student, Computer Science Department, PUC-Rio.</p>",
 ["published_at"] = 1183578360,
 ["image"] = "",
 ["title"] = "Fabio Mascarenhas",
 ["comment_status"] = "closed",
 ["section_id"] = 10,
 ["published"] = true,
 ["abstract"] = "",
 ["user_id"] = 1}
rec = t:new(rec)
rec:save(true)

-- Record 19

local rec = {
 ["id"] = 19,
 ["external_url"] = "",
 ["body"] = "<p><a href=\"mailto:tampo_8@yahoo.com\">Sérgio Medeiros</a>, PhD Student, Computer Science Department, PUC-Rio.</p>",
 ["published_at"] = 1183491960,
 ["image"] = "",
 ["title"] = "Sérgio Medeiros",
 ["comment_status"] = "closed",
 ["section_id"] = 10,
 ["published"] = true,
 ["abstract"] = "",
 ["user_id"] = 1}
rec = t:new(rec)
rec:save(true)

-- Record 20

local rec = {
 ["id"] = 20,
 ["external_url"] = "",
 ["body"] = "<p><a href=\"http://www.inf.puc-rio.br/~abarros/\">Alexandra Barros</a>, MSc Student, Computer Science Department, PUC-Rio.</p>",
 ["published_at"] = 1183578420,
 ["image"] = "",
 ["title"] = "Alexandra Barros",
 ["comment_status"] = "closed",
 ["section_id"] = 11,
 ["published"] = true,
 ["abstract"] = "",
 ["user_id"] = 1}
rec = t:new(rec)
rec:save(true)

-- Record 21

local rec = {
 ["id"] = 21,
 ["external_url"] = "",
 ["body"] = "<p><a href=\"http://www.inf.puc-rio.br/~hisham/\">Hisham Muhammad</a>, MSc Computer Science, PUC-Rio.</p>",
 ["published_at"] = 1183492020,
 ["image"] = "",
 ["title"] = "Hisham Muhammad",
 ["comment_status"] = "closed",
 ["section_id"] = 11,
 ["published"] = true,
 ["abstract"] = "",
 ["user_id"] = 1}
rec = t:new(rec)
rec:save(true)

-- Record 22

local rec = {
 ["id"] = 22,
 ["external_url"] = "",
 ["body"] = "<p>Ana Lúcia de Moura, PhD Computer Science, PUC-Rio.</p>",
 ["published_at"] = 1183405620,
 ["image"] = "",
 ["title"] = "Ana Lúcia de Moura",
 ["comment_status"] = "closed",
 ["section_id"] = 11,
 ["published"] = true,
 ["abstract"] = "",
 ["user_id"] = 1}
rec = t:new(rec)
rec:save(true)

-- Table comment

local t = mapper:new('comment')

-- Table user

local t = mapper:new('user')

-- Record 1

local rec = {
 ["id"] = 1,
 ["name"] = "Fabio Mascarenhas",
 ["login"] = "mascarenhas@acm.org",
 ["password"] = "password"}
rec = t:new(rec)
rec:save(true)

-- Table section

local t = mapper:new('section')

-- Record 1

local rec = {
 ["id"] = 1,
 ["description"] = "",
 ["title"] = "Home",
 ["tag"] = "home"}
rec = t:new(rec)
rec:save(true)

-- Record 2

local rec = {
 ["id"] = 2,
 ["description"] = "",
 ["title"] = "Projects",
 ["tag"] = "menu-projects"}
rec = t:new(rec)
rec:save(true)

-- Record 3

local rec = {
 ["id"] = 3,
 ["description"] = "",
 ["title"] = "Publications",
 ["tag"] = "menu-publications"}
rec = t:new(rec)
rec:save(true)

-- Record 4

local rec = {
 ["id"] = 4,
 ["description"] = "",
 ["title"] = "People",
 ["tag"] = "menu-people"}
rec = t:new(rec)
rec:save(true)

-- Record 5

local rec = {
 ["id"] = 5,
 ["description"] = "",
 ["title"] = "Related Links",
 ["tag"] = "links"}
rec = t:new(rec)
rec:save(true)

-- Record 6

local rec = {
 ["id"] = 6,
 ["description"] = "",
 ["title"] = "Papers",
 ["tag"] = "pubs-papers"}
rec = t:new(rec)
rec:save(true)

-- Record 7

local rec = {
 ["id"] = 7,
 ["description"] = "",
 ["title"] = "Dissertations and Theses",
 ["tag"] = "pubs-theses"}
rec = t:new(rec)
rec:save(true)

-- Record 8

local rec = {
 ["id"] = 8,
 ["description"] = "",
 ["title"] = "Drafts",
 ["tag"] = "pubs-drafts"}
rec = t:new(rec)
rec:save(true)

-- Record 9

local rec = {
 ["id"] = 9,
 ["description"] = "",
 ["title"] = "Coordinator",
 ["tag"] = "people-coordinator"}
rec = t:new(rec)
rec:save(true)

-- Record 10

local rec = {
 ["id"] = 10,
 ["description"] = "",
 ["title"] = "Current Members",
 ["tag"] = "people-current"}
rec = t:new(rec)
rec:save(true)

-- Record 11

local rec = {
 ["id"] = 11,
 ["description"] = "",
 ["title"] = "Former Members",
 ["tag"] = "people-former"}
rec = t:new(rec)
rec:save(true)
