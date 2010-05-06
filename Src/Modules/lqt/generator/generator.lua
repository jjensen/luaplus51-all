#!/usr/bin/lua

--[[

Copyright (c) 2007-2009 Mauro Iazzi
Copyright (c)      2008 Peter Kümmel

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

--]]

local osseparator = package.config:sub(1,1)

local path = string.match(arg[0], '(.*'..osseparator..')[^%'..osseparator..']+') or ''
if path == "" then
	--- remove script name
	path = string.sub(arg[0], 1, #arg[0] - #'generator.lua')
end

local filename = nil
local dirname = nil
local module_name = nil
local typefiles = {}
local filterfiles = {}
local output_includes = {
	'lqt_common.hpp',
}
local output_path = '.'

do
	local i = 1
	while select(i, ...) do
		local argi = select(i, ...)
		if argi=='-n' then
			i = i + 1
			module_name = select(i, ...)
		elseif argi=='-i' then
			i = i + 1
			table.insert(output_includes, (select(i, ...)))
		elseif argi=='-t' then
			i = i + 1
			table.insert(typefiles, (select(i, ...)))
		elseif argi=='-f' then
			i = i + 1
			table.insert(filterfiles, (select(i, ...)))
		elseif argi=='-D' then
			i = i + 1
			output_path = select(i, ...)
		else
			filename = filename and error'duplicate filename' or argi
		end
		i = i + 1
	end
end

if not output_path:match('[/\\]$') then output_path = output_path .. '/' end

local my_includes = ''
for _, i in ipairs(output_includes) do
	if string.match(i, '^<.+>$') then
		my_includes = my_includes .. '#include '..i..'\n'
	elseif string.match(i, '^".+"$') then
		my_includes = my_includes .. '#include '..i..'\n'
	else
		my_includes = my_includes .. '#include "'..i..'"\n'
	end
end
output_includes = my_includes .. '\n'

local readfile = function(fn)
	local f = assert(io.open(fn))
	local s = f:read'*a'
	f:close()
	return s
end

local fprint = function(f)
	return function(...)
		for i = 1, select('#',...) do
			f:write((i==1) and '' or '\t', tostring(select(i,...)))
		end
		f:write'\n'
		f:flush()
	end
end

local _src = '_src'..osseparator
local debug = fprint(io.stderr)
local print_enum = fprint(assert(io.open(output_path..module_name..'_enum.cpp', 'w')))
local print_slot_h = fprint(assert(io.open(output_path..module_name..'_slot.hpp', 'w')))
local print_slot_c = fprint(assert(io.open(output_path..module_name..'_slot.cpp', 'w')))

local xmlstream, idindex = dofile(path..'xml.lua')(readfile(filename))

dofile(path..'classes.lua') -- this should become a require at some point

----------------------------------------------------------------------------------

local copy_functions = function(index)
	local ret = {}
	for e in pairs(index) do
		if e.label:match'^Function' then
			e.label = 'Function'
			ret[e] = true
		end
	end
	return ret
end


local fix_arguments = function(all)
	local fullnames = {}
	for e in pairs(all or {}) do
		if e.xarg.fullname then fullnames[e.xarg.fullname] = true end
	end
	for a in pairs(all) do
		if a.label=='Argument'
			and a.xarg.default=='1'
			and (not string.match(a.xarg.defaultvalue, '^[-+]?%d+%.?%d*[L]?$'))
			and (not string.match(a.xarg.defaultvalue, '^".*"$'))
			and a.xarg.defaultvalue~='true'
			and a.xarg.defaultvalue~='false'
			and (not string.match(a.xarg.defaultvalue, '^0[xX]%d+$')) then
			local dv, call = string.match(a.xarg.defaultvalue, '(.-)(%(%))')
			dv = dv or a.xarg.defaultvalue
			call = call or ''
			local context = a.xarg.context
			while not fullnames[context..'::'..dv] and context~='' do
				context = string.match(context, '^(.*)::') or ''
			end
			if fullnames[context..'::'..dv] then
				a.xarg.defaultvalue = context..'::'..dv..call
			elseif fullnames[dv] then
				a.xarg.defaultvalue = dv..call
			else
				a.xarg.default = nil
				a.xarg.defaultvalue = nil
			end
		end
	end
	return all
end

local fix_functions = function(index)
	for f in pairs(index) do
		local args = {}
		for i, a in ipairs(f) do
			-- avoid bogus 'void' arguments
			if a.xarg.type_name=='void' and i==1 and f[2]==nil then break end
			if a.label=='Argument' then
				table.insert(args, a)
			end
		end
		f.arguments = args
		f.return_type = f.xarg.type_name
		if f.xarg.type_name=='void' then
			f.return_type = nil
		end
	end
	return index
end

local copy_enums = function(index)
	local ret = {}
	for e in pairs(index) do
		if e.label=='Enum'
			and not string.match(e.xarg.fullname, '%b<>')
			and e.xarg.access=='public' then
			ret[e] = true
		end
	end
	return ret
end

local fill_enums = function(index)
	for e in pairs(index) do
		local values = {}
		for _, v in ipairs(e) do
			if v.label=='Enumerator' then
				table.insert(values, v)
			end
		end
		e.values = values
	end
	return index
end

local class_is_public = function (fullnames, c)
	repeat
		if c.xarg.access~='public' then return false end
		if c.xarg.member_of_class then
			local p = fullnames[c.xarg.member_of_class]
			assert(p, 'member_of_class should exist')
			assert(p.label=='Class', 'member_of_class should be a class')
			c = fullnames[c.xarg.member_of_class]
		else
			break
		end
	until true
	return true
end

local copy_classes = function(index)
	local ret = {}
	local fullnames = {}
	for k,v in pairs(index) do
		if k.label=='Class' then fullnames[k.xarg.fullname] = k end
	end
	for e in pairs(index) do
		if e.label=='Class'
			and class_is_public(fullnames, e)
			and not e.xarg.fullname:match'%b<>' then
			ret[e] = true
		elseif e.label=='Class'
			and not e.xarg.fullname:match'%b<>' then
			--print('class', e.xarg.fullname, 'rejected because not public')
		end
	end
	return ret
end

local get_qobjects = function(index)
	local classes = {}
	for c in pairs(index) do
		classes[c.xarg.fullname] = c
	end
	local is_qobject
	is_qobject = function(c)
		if c==nil then return false end
		if c.qobject then return true end
		if c.xarg.fullname=='QObject' then
			c.qobject = true
			return true
		end
		for b in string.gmatch(c.xarg.bases or '', '([^;]+);') do
			local base = classes[b]
			if is_qobject(base) then
				--debug(c.xarg.fullname, "is a QObject")
				c.qobject = true
				return true
			end
		end
		return false
	end
	for c in pairs(index) do
		local qobj = is_qobject(c)
	end
	return index
end

local fill_virtuals = function(index)
	local classes = {}
	for c in pairs(index) do
		classes[c.xarg.fullname] = c
	end
	local get_virtuals
	get_virtuals = function(c)
		local ret = {}
		for _, f in ipairs(c) do
			if f.label=='Function' and f.xarg.virtual=='1' then
				local n = string.match(f.xarg.name, '~') or f.xarg.name
				if n~='~' and n~='metaObject' then ret[n] = f end
			end
		end
		for b in string.gmatch(c.xarg.bases or '', '([^;]+);') do
			local base = classes[b]
			if type(base)=='table' then
				local bv = get_virtuals(base)
				for n, f in pairs(bv) do
					if not ret[n] then ret[n] = f end
				end
			end
		end
		for _, f in ipairs(c) do
			if f.label=='Function'
				and f.xarg.access~='private'
				and (ret[string.match(f.xarg.name, '~') or f.xarg.name]) then
				f.xarg.virtual = '1'
				local n = string.match(f.xarg.name, '~')or f.xarg.name
				ret[n] = f
			end
		end
		return ret
	end
	for c in pairs(index) do
		c.virtuals = get_virtuals(c)
		for _, f in pairs(c.virtuals) do
			if f.xarg.abstract=='1' then c.abstract=true break end
		end
	end
	return index
end

local distinguish_methods = function(index)
	for c in pairs(index) do
		local construct, destruct, normal = {}, nil, {}
		local n = c.xarg.name
		local copy = nil
		for _, f in ipairs(c) do
			if n==f.xarg.name then
				table.insert(construct, f)
			elseif f.xarg.name:match'~' then
				destruct = f
			else
				if (not string.match(f.xarg.name, '^operator%W'))
					and (not f.xarg.member_template_parameters)
					and (not f.xarg.friend) then
					table.insert(normal, f)
				end
			end
		end
		c.constructors = construct
		c.destructor = destruct
		c.methods = normal
	end
	return index
end

local fill_public_destr = function(index)
	local classes = {}
	for c in pairs(index) do
		classes[c.xarg.fullname] = c
	end
	local destr_is_public
	destr_is_public = function(c)
		if c.destructor then
			return c.destructor.xarg.access=='public'
		else
			for b in string.gmatch(c.xarg.bases or '', '([^;]+);') do
				local base = classes[b]
				if base and not destr_is_public(base) then
					return false
				end
			end
			return true
		end
	end
	for c in pairs(index) do
		c.public_destr = destr_is_public(c)
	end
	return index
end

local fill_copy_constructor = function(index)
	local classes = {}
	for c in pairs(index) do
		classes[c.xarg.name] = c
	end
	for c in pairs(index) do
		local copy = nil
		for _, f in ipairs(c.constructors) do
			if #(f.arguments)==1
				and f.arguments[1].xarg.type_name==c.xarg.fullname..' const&' then
				copy = f
				break
			end
		end
		c.copy_constructor = copy
	end
	local copy_constr_is_public
	copy_constr_is_public = function(c)
		if c.copy_constructor then
			return (c.copy_constructor.xarg.access=='public')
			or (c.copy_constructor.xarg.access=='protected')
		else
			local ret = nil
			for b in string.gmatch(c.xarg.bases or '', '([^;]+);') do
				local base = classes[b]
				if base and not copy_constr_is_public(base) then
					return false
				end
			end
			return true
		end
	end
	for c in pairs(index) do
		c.public_constr = copy_constr_is_public(c)
	end
	return index
end

local fill_typesystem_with_enums = function(enums, types)
	local etype = function(en)
		return {
			push = function(n)
				return 'lqtL_pushenum(L, '..n..', "'..string.gsub(en, '::', '.')..'")', 1
			end,
			get = function(n)
				return 'static_cast<'..en..'>'
				..'(lqtL_toenum(L, '..n..', "'..string.gsub(en, '::', '.')..'"))', 1
			end,
			test = function(n)
				return 'lqtL_isenum(L, '..n..', "'..string.gsub(en, '::', '.')..'")', 1
			end,
			onstack = string.gsub(en, '::', '.')..',',
		}
	end
	local ret = {}
	for e in pairs(enums) do
		if types[e.xarg.fullname]==nil then
			ret[e] = true
			types[e.xarg.fullname] = etype(e.xarg.fullname)
		else
			--io.stderr:write(e.xarg.fullname, ': already present\n')
		end
	end
	return ret
end

local put_class_in_filesystem = lqt.classes.insert
local fill_typesystem_with_classes = function(classes, types)
	local ret = {}
	for c in pairs(classes) do
		ret[c] = put_class_in_filesystem(c.xarg.fullname, types) --, true)
	end
	return ret
end

local argument_name = function(tn, an)
	local ret
	if string.match(tn, '%(%*%)') then
		ret = string.gsub(tn, '%(%*%)', '(*'..an..')', 1)
	elseif string.match(tn, '%[.*%]') then
		ret = string.gsub(tn, '%s*(%[.*%])', ' '..an..'%1')
	else
		ret = tn .. ' ' .. an
	end
	return ret
end

local fill_wrapper_code = function(f, types)
	if f.wrapper_code then return f end
	local stackn, argn = 1, 1
	local stack_args, defects = '', 0
	local wrap, line = '  int oldtop = lua_gettop(L);\n', ''
	if f.xarg.abstract then return nil end
	if f.xarg.member_of_class and f.xarg.static~='1' then
		if not types[f.xarg.member_of_class..'*'] then return nil end -- print(f.xarg.member_of_class) return nil end
		stack_args = stack_args .. types[f.xarg.member_of_class..'*'].onstack
		defects = defects + 7 -- FIXME: arbitrary
		if f.xarg.constant=='1' then
			defects = defects + 8 -- FIXME: arbitrary
		end
		local sget, sn = types[f.xarg.member_of_class..'*'].get(stackn)
		wrap = wrap .. '  ' .. f.xarg.member_of_class .. '* self = ' .. sget .. ';\n'
		stackn = stackn + sn
		wrap = wrap .. [[
  if (NULL==self) {
    lua_pushstring(L, "this pointer is NULL");
    lua_error(L);
  }
]]
		--print(sget, sn)
		line = 'self->'..f.xarg.fullname..'('
	else
		line = f.xarg.fullname..'('
	end
	for i, a in ipairs(f.arguments) do
		if not types[a.xarg.type_name] then return nil end
		local aget, an, arg_as = types[a.xarg.type_name].get(stackn)
		stack_args = stack_args .. types[a.xarg.type_name].onstack
		if types[a.xarg.type_name].defect then defects = defects + types[a.xarg.type_name].defect end
		wrap = wrap .. '  ' .. argument_name(arg_as or a.xarg.type_name, 'arg'..argn) .. ' = '
		if a.xarg.default=='1' and an>0 then
			wrap = wrap .. 'lua_isnoneornil(L, '..stackn..')'
			for j = stackn+1,stackn+an-1 do
				wrap = wrap .. ' && lua_isnoneornil(L, '..j..')'
			end
			local dv = a.xarg.defaultvalue
			wrap = wrap .. ' ? static_cast< ' .. a.xarg.type_name .. ' >(' .. dv .. ') : '
		end
		wrap = wrap .. aget .. ';\n'
		line = line .. (argn==1 and 'arg' or ', arg') .. argn
		stackn = stackn + an
		argn = argn + 1
	end
	line = line .. ')'
	-- FIXME: hack follows for constructors
	if f.calling_line then line = f.calling_line end
	if f.return_type then line = f.return_type .. ' ret = ' .. line end
	wrap = wrap .. '  ' .. line .. ';\n  lua_settop(L, oldtop);\n' -- lua_pop(L, '..stackn..');\n'
	if f.return_type then
		if not types[f.return_type] then return nil end
		local rput, rn = types[f.return_type].push'ret'
		wrap = wrap .. '  luaL_checkstack(L, '..rn..', "cannot grow stack for return value");\n'
		wrap = wrap .. '  '..rput..';\n  return '..rn..';\n'
	else
		wrap = wrap .. '  return 0;\n'
	end
	f.wrapper_code = wrap
	f.stack_arguments = stack_args
	f.defects = defects
	return f
end

local fill_test_code = function(f, types)
	local stackn = 1
	local test = ''
	if f.xarg.member_of_class and f.xarg.static~='1' then
		if not types[f.xarg.member_of_class..'*'] then return nil end -- print(f.xarg.member_of_class) return nil end
		local stest, sn = types[f.xarg.member_of_class..'*'].test(stackn)
		test = test .. ' && ' .. stest
		stackn = stackn + sn
	end
	for i, a in ipairs(f.arguments) do
		if not types[a.xarg.type_name] then return nil end -- print(a.xarg.type_name) return nil end
		local atest, an = types[a.xarg.type_name].test(stackn)
		if a.xarg.default=='1' and an>0 then
			test = test .. ' && (lqtL_missarg(L, ' .. stackn .. ', ' .. an .. ') || '
			test = test .. atest .. ')'
		else
			test = test .. ' && ' .. atest
		end
		stackn = stackn + an
	end
	-- can't make use of default values if I fix number of args
	test = '(lua_gettop(L)<' .. stackn .. ')' .. test
	f.test_code = test
	return f
end

local fill_wrappers = function(functions, types)
	local ret = {}
	for f in pairs(functions) do
		f = fill_wrapper_code(f, types)
		if f then
			f = assert(fill_test_code(f, types), f.xarg.fullname) -- MUST pass
			ret[f] = true
			--local out = 'extern "C" LQT_EXPORT  int lqt_bind'..f.xarg.id..' (lua_State *L) {\n'
			--.. f.wrapper_code .. '}\n'
			--print(out)
		end
	end
	return ret
end

local make_pushlines = function(args, types)
	local pushlines, stack = '', 0
	for i, a in ipairs(args) do
		if not types[a.xarg.type_name] then return nil end
		local apush, an = types[a.xarg.type_name].push('arg'..i)
		pushlines = pushlines .. '    ' .. apush .. ';\n'
		stack = stack + an
	end
	return pushlines, stack
end

local virtual_overload = function(v, types)
	local ret = ''
	if v.virtual_overload then return v end
	-- make return type
	if v.return_type and not types[v.return_type] then return nil end
	local rget, rn = '', 0
	if v.return_type then rget, rn, ret_as = types[v.return_type].get'oldtop+1' end
	local retget = (v.return_type and argument_name(ret_as or v.return_type, 'ret')
	.. ' = ' .. rget .. ';' or '') .. 'lua_settop(L, oldtop);return'
	.. (v.return_type and ' ret' or '')
	-- make argument push
	local pushlines, stack = make_pushlines(v.arguments, types)
	if not pushlines then return nil end
	-- make lua call
	local luacall = 'lqtL_pcall(L, '..(stack+1)..', '..rn..', 0)'
	-- make prototype and fallback
	local proto = (v.return_type or 'void')..' ;;'..v.xarg.name..' ('
	local fallback = ''
	for i, a in ipairs(v.arguments) do
		proto = proto .. (i>1 and ', ' or '')
		.. argument_name(a.xarg.type_name, 'arg'..i)
		fallback = fallback .. (i>1 and ', arg' or 'arg') .. i
	end
	proto = proto .. ')' .. (v.xarg.constant=='1' and ' const' or '')
	fallback = (v.return_type and 'return this->' or 'this->')
	.. v.xarg.fullname .. '(' .. fallback .. ');\n}\n'
	local pre_ret_type_str = ""
	local ret_type_str = "    return;\n"
	if v.return_type  and  v.return_type ~= 'void' then
		pre_ret_type_str = '    ' .. v.return_type .. ' save = ' .. types[v.return_type].get(-1) .. ';\n'
		ret_type_str = '    return save;\n'
	end
	ret = proto .. [[ {
  int oldtop = lua_gettop(L);
  lqtL_pushudata(L, this, "]]..string.gsub(v.xarg.member_of_class, '::', '.')..[[*");
  lqtL_getoverload(L, -1, "]]..v.xarg.name..[[");
  if (lua_isfunction(L, -1)) {
    lua_insert(L, -2);
]] .. pushlines .. [[
    if (!]]..luacall..[[) {
      lua_error(L);
    }
]] .. pre_ret_type_str .. [[
    lua_settop(L, oldtop);
]] .. ret_type_str .. [[
  }
  lua_settop(L, oldtop);
  ]] .. fallback
	v.virtual_overload = ret
	v.virtual_proto = string.gsub(proto, ';;', '', 1)
	return v
end

local fill_shell_class = function(c, types)
	local shellname = 'lqt_shell_'..string.gsub(c.xarg.fullname, '::', '_LQT_')
	local shell = 'class LQT_EXPORT ' .. shellname .. ' : public ' .. c.xarg.fullname .. ' {\npublic:\n'
	shell = shell .. '  lua_State *L;\n'
	for _, constr in ipairs(c.constructors) do
		if constr.xarg.access~='private' then
			local cline = '  '..shellname..' (lua_State *l'
			local argline = ''
			for i, a in ipairs(constr.arguments) do
				cline = cline .. ', ' .. argument_name(a.xarg.type_name, 'arg'..i)
				argline = argline .. (i>1 and ', arg' or 'arg') .. i
			end
			cline = cline .. ') : ' .. c.xarg.fullname
				.. '(' .. argline .. '), L(l) '
				.. '{ lqtL_register(L, this); }\n'
			shell = shell .. cline
		end
	end
	if c.copy_constructor==nil and c.public_constr then
		local cline = '  '..shellname..' (lua_State *l, '..c.xarg.fullname..' const& arg1)'
		cline = cline .. ' : ' .. c.xarg.fullname .. '(arg1), L(l) {}\n'
		shell = shell .. cline
	end
	for i, v in pairs(c.virtuals) do
		if v.xarg.access~='private' then
			if v.virtual_proto then shell = shell .. '  virtual ' .. v.virtual_proto .. ';\n' end
		end
	end
	shell = shell .. '  ~'..shellname..'() { lqtL_unregister(L, this); }\n'
	if c.shell and c.qobject then
		shell = shell .. '  static QMetaObject staticMetaObject;\n'
		shell = shell .. '  virtual const QMetaObject *metaObject() const;\n'
		shell = shell .. '  virtual int qt_metacall(QMetaObject::Call, int, void **);\n'
		shell = shell .. 'private:\n'
		shell = shell .. '      Q_DISABLE_COPY('..shellname..');\n'
	end
	shell = shell .. '};\n'
	c.shell_class = shell
	return c
end

local fill_virtual_overloads = function(classes, types)
	for c in pairs(classes) do
		for i, v in pairs(c.virtuals) do
			if v.xarg.access~='private' then
				local vret = virtual_overload(v, types)
			end
		end
	end
	return classes
end

local fill_shell_classes = function(classes, types)
	local ret = {}
	for c in pairs(classes) do
		if c.shell then
			c = fill_shell_class(c, types)
			if c then ret[c] = true end
		end
		ret[c] = true
	end
	return ret
end

local print_shell_classes = function(classes)
	local fhead = nil
	for c in pairs(classes) do
		if fhead then fhead:close() end
		local n = string.gsub(c.xarg.fullname, '::', '_LQT_')
		fhead = assert(io.open(output_path..module_name..'_head_'..n..'.hpp', 'w'))
		local print_head = function(...)
			fhead:write(...)
			fhead:write'\n'
		end
		print_head('#ifndef LQT_HEAD_'..n)
		print_head('#define LQT_HEAD_'..n)
		print_head(output_includes)
		--print_head('#include <'..string.match(c.xarg.fullname, '^[^:]+')..'>')
		print_head''
		if c.shell then
			print_head('#include "'..module_name..'_slot.hpp'..'"\n\n')
			if c then
				print_head(c.shell_class)
			else
				--io.stderr:write(c.fullname, '\n')
			end
		end
		print_head('#endif // LQT_HEAD_'..n)
	end
	if fhead then fhead:close() end
	return classes
end

local print_virtual_overloads = function(classes)
	for c in pairs(classes) do
		if c.shell then
			local vo = ''
			local shellname = 'lqt_shell_'..string.gsub(c.xarg.fullname, '::', '_LQT_')
			for _,v in pairs(c.virtuals) do
				if v.virtual_overload then
					vo = vo .. string.gsub(v.virtual_overload, ';;', shellname..'::', 1)
				end
			end
			c.virtual_overloads = vo
		end
	end
	return classes
end

local print_wrappers = function(index)
	for c in pairs(index) do
		local meta = {}
		local wrappers = ''
		for _, f in ipairs(c.methods) do
			-- FIXME: should we really discard virtual functions?
			-- if the virtual overload in the shell uses rawget
			-- on the environment then we can leave these in the
			-- metatable
			if f.wrapper_code and f.xarg.abstract~='1' then
				local out = 'static int lqt_bind'..f.xarg.id
				..' (lua_State *L) {\n'.. f.wrapper_code .. '}\n'
				if f.xarg.access=='public' then
					--print_meta(out)
					wrappers = wrappers .. out .. '\n'
					meta[f] = f.xarg.name
				end
			end
		end
		if not c.abstract then
			for _, f in ipairs(c.constructors) do
				if f.wrapper_code then
					local out = 'static int lqt_bind'..f.xarg.id
					    ..' (lua_State *L) {\n'.. f.wrapper_code .. '}\n'
					if f.xarg.access=='public' then
						--print_meta(out)
						wrappers = wrappers .. out .. '\n'
						meta[f] = 'new'
					end
				end
			end
		end
		--local shellname = 'lqt_shell_'..string.gsub(c.xarg.fullname, '::', '_LQT_')
		local lua_name = string.gsub(c.xarg.fullname, '::', '.')
		local out = 'static int lqt_delete'..c.xarg.id..' (lua_State *L) {\n'
		out = out ..'  '..c.xarg.fullname..' *p = static_cast<'
			..c.xarg.fullname..'*>(lqtL_toudata(L, 1, "'..lua_name..'*"));\n'
		if c.public_destr then
			out = out .. '  if (p) delete p;\n'
		end
		out = out .. '  lqtL_eraseudata(L, 1, "'..lua_name..'*");\n  return 0;\n}\n'
		--print_meta(out)
		wrappers = wrappers .. out .. '\n'
		c.meta = meta
		c.wrappers = wrappers
	end
	return index
end

local print_metatable = function(c)
	local methods = {}
	local wrappers = c.wrappers
	for m, n in pairs(c.meta) do
		methods[n] = methods[n] or {}
		table.insert(methods[n], m)
	end
	for n, l in pairs(methods) do
		local duplicates = {}
		for _, f in ipairs(l) do
			local itisnew = true
			for sa, g in pairs(duplicates) do
				if sa==f.stack_arguments then
					--debug("function equal: ", f.xarg.fullname, f.stack_arguments, sa, f.defects, g.defects)
					if f.defects<g.defects then
					else
						itisnew = false
					end
				elseif string.match(sa, "^"..f.stack_arguments) then -- there is already a version with more arguments
					--debug("function superseded: ", f.xarg.fullname, f.stack_arguments, sa, f.defects, g.defects)
				elseif string.match(f.stack_arguments, '^'..sa) then -- there is already a version with less arguments
					--debug("function superseding: ", f.xarg.fullname, f.stack_arguments, sa, f.defects, g.defects)
				end
			end
			if itisnew then
				duplicates[f.stack_arguments] = f
			end
		end
		--[[
		local numinitial = 0
		local numfinal = 0
		for sa, f in pairs(l) do
			numinitial = numinitial + 1
		end
		for sa, f in pairs(duplicates) do
			numfinal = numfinal + 1
		end
		if numinitial-numfinal>0 then debug(c.xarg.fullname, "suppressed:", numinitial-numfinal) end
		--]]
		methods[n] = duplicates
	end
	for n, l in pairs(methods) do
		local disp = 'static int lqt_dispatcher_'..n..c.xarg.id..' (lua_State *L) {\n'
		for _, f in pairs(l) do
			disp = disp..'  if ('..f.test_code..') return lqt_bind'..f.xarg.id..'(L);\n'
		end
		disp = disp .. '  lua_settop(L, 0);\n'
		disp = disp .. '  lua_pushstring(L, "'..c.xarg.fullname..'::'..n..': incorrect or extra arguments");\n'
		disp = disp .. '  return lua_error(L);\n}\n'
		--print_meta(disp)
		wrappers = wrappers .. disp .. '\n'
	end
	local metatable = 'static luaL_Reg lqt_metatable'..c.xarg.id..'[] = {\n'
	for n, l in pairs(methods) do
		metatable = metatable .. '  { "'..n..'", lqt_dispatcher_'..n..c.xarg.id..' },\n'
	end
	metatable = metatable .. '  { "delete", lqt_delete'..c.xarg.id..' },\n'
	metatable = metatable .. '  { 0, 0 },\n};\n'
	--print_meta(metatable)
	wrappers = wrappers .. metatable .. '\n'
	local bases = ''
	for b in string.gmatch(c.xarg.bases_with_attributes or '', '([^;]*);') do
		if not string.match(b, '^virtual') then
			b = string.gsub(b, '^[^%s]* ', '')
			bases = bases .. '  {"'..string.gsub(b,'::','.')..'*", (char*)(void*)static_cast<'..b..'*>(('..c.xarg.fullname..'*)1)-(char*)1},\n'
		end
	end
	bases = 'static lqt_Base lqt_base'..c.xarg.id..'[] = {\n'..bases..'  {NULL, 0}\n};\n'
	--print_meta(bases)
	wrappers = wrappers .. bases .. '\n'
	c.wrappers = wrappers
	return c
end

local print_metatables = function(classes)
	for c in pairs(classes) do
		print_metatable(c)
	end
	return classes
end

local cpp_files = {}

local print_single_class = function(c)
	local n = string.gsub(c.xarg.fullname, '::', '_LQT_')
	local lua_name = string.gsub(c.xarg.fullname, '::', '.')
	local cppname = module_name..'_meta_'..n..'.cpp'
	table.insert(cpp_files, cppname);
	local fmeta = assert(io.open(output_path..cppname, 'w'))
	local print_meta = function(...)
		fmeta:write(...)
		fmeta:write'\n'
	end
	print_meta('#include "'..module_name..'_head_'..n..'.hpp'..'"\n\n')
	print_meta(c.wrappers)
	if c.virtual_overloads then
		print_meta(c.virtual_overloads)
	end
	print_meta('extern "C" LQT_EXPORT int luaopen_'..n..' (lua_State *L) {')
	print_meta('\tlqtL_createclass(L, "'
		..lua_name..'*", lqt_metatable'
		..c.xarg.id..', lqt_base'
		..c.xarg.id..');')
	print_meta'\treturn 0;'
	print_meta'}'
	print_meta''
	if c.shell and c.qobject then
		print_meta([[
#include <QDebug>

QMetaObject lqt_shell_]]..n..[[::staticMetaObject;

const QMetaObject *lqt_shell_]]..n..[[::metaObject() const {
        //int oldtop = lua_gettop(L);
        lqtL_pushudata(L, this, "]]..c.xarg.fullname..[[*");
        lua_getfield(L, -1, LQT_OBJMETASTRING);
        if (lua_isnil(L, -1)) {
                lua_pop(L, 2);
                return &]]..c.xarg.fullname..[[::staticMetaObject;
        }
        lua_getfield(L, -2, LQT_OBJMETADATA);
        lqtL_touintarray(L);
        //qDebug() << "copying qmeta object for slots in ]]..c.xarg.fullname..[[";
        lqt_shell_]]..n..[[::staticMetaObject.d.superdata = &]]..c.xarg.fullname..[[::staticMetaObject;
        lqt_shell_]]..n..[[::staticMetaObject.d.stringdata = lua_tostring(L, -2);
        lqt_shell_]]..n..[[::staticMetaObject.d.data = (uint*)lua_touserdata(L, -1);
        lqt_shell_]]..n..[[::staticMetaObject.d.extradata = 0; // slot_metaobj->d.extradata;
        lua_setfield(L, LUA_REGISTRYINDEX, LQT_OBJMETADATA);
        lua_setfield(L, LUA_REGISTRYINDEX, LQT_OBJMETASTRING);
        lua_pop(L, 1);
        //qDebug() << (lua_gettop(L) - oldtop);
        return &lqt_shell_]]..n..[[::staticMetaObject;
}

int lqt_shell_]]..n..[[::qt_metacall(QMetaObject::Call call, int index, void **args) {
        //qDebug() << "fake calling!";
        index = ]]..c.xarg.fullname..[[::qt_metacall(call, index, args);
        if (index < 0) return index;
        return lqtL_qt_metacall(L, this, lqtSlotAcceptor_]]..module_name..[[, call, "]]..c.xarg.fullname..[[*", index, args);
}
]])
	end
	fmeta:close()
end

local print_merged_build = function()
	local path = output_path
	local mergename = module_name..'_merged_build'
	local merged = assert(io.open(path..mergename..'.cpp', 'w'))
	for _, p in ipairs(cpp_files) do
		merged:write('#include "'..p..'"\n')
	end
	local pro_file = assert(io.open(path..mergename..'.pro', 'w'))

	local print_pro= function(...)
		pro_file:write(...)
		pro_file:write'\n'
	end
	print_pro('TEMPLATE = lib')
	print_pro('TARGET = '..module_name)
	print_pro('INCLUDEPATH += .')
	print_pro('HEADERS += '..module_name..'_slot.hpp')
	print_pro('SOURCES += ../common/lqt_common.cpp \\')
	print_pro('          ../common/lqt_qt.cpp \\')
	print_pro('          '..module_name..'_enum.cpp \\')
	print_pro('          '..module_name..'_meta.cpp \\')
	print_pro('          '..module_name..'_slot.cpp \\')
	print_pro('          '..mergename..'.cpp')
end

local print_class_list = function(classes)
	local qobject_present = false
	local big_picture = {}
	local type_list_t = {}
	for c in pairs(classes) do
		local n = string.gsub(c.xarg.fullname, '::', '_LQT_')
		if n=='QObject' then qobject_present = true end
		print_single_class(c)
		table.insert(big_picture, 'luaopen_'..n)
		table.insert(type_list_t, 'add_class(\''..c.xarg.fullname..'\', types)\n')
	end

	local type_list_f = assert(io.open(output_path..module_name..'_types.lua', 'w'))
	type_list_f:write([[
#!/usr/bin/lua
local types = (...) or {}
local add_class = lqt.classes.insert or error('module lqt.classes not loaded')
]])
	for k, v in ipairs(type_list_t) do
		type_list_f:write(v)
	end
	type_list_f:write('return types\n')
	type_list_f:close()

	print_merged_build()
	if fmeta then fmeta:close() end
	fmeta = assert(io.open(output_path..module_name..'_meta.cpp', 'w'))
	local print_meta = function(...)
		fmeta:write(...)
		fmeta:write'\n'
	end
	print_meta()
	print_meta('#include "lqt_common.hpp"')
	print_meta('#include "'..module_name..'_slot.hpp'..'"\n\n')
	for _, p in ipairs(big_picture) do
		print_meta('extern "C" LQT_EXPORT int '..p..' (lua_State *);')
	end
	print_meta('void lqt_create_enums_'..module_name..' (lua_State *);')
	print_meta('extern "C" LQT_EXPORT int luaopen_'..module_name..' (lua_State *L) {')
	for _, p in ipairs(big_picture) do
		print_meta('\t'..p..'(L);')
	end
	print_meta('\tlqt_create_enums_'..module_name..'(L);')
	if qobject_present then
		print_meta('\tlua_getfield(L, LUA_REGISTRYINDEX, "QObject*");')
		print_meta('\tlua_pushstring(L, "__addmethod");')
		print_meta('\tlqtL_pushaddmethod(L);')
		print_meta('\tlua_rawset(L, -3);')
	end
	print_meta('\t//lua_pushlightuserdata(L, (void*)&LqtSlotAcceptor::staticMetaObject);')
	print_meta('\t//lua_setfield(L, LUA_REGISTRYINDEX, LQT_METAOBJECT);')
	print_meta('\t//lqtL_passudata(L, (void*)(new LqtSlotAcceptor(L)), "QObject*");')
	print_meta('\t//lua_setfield(L, LUA_REGISTRYINDEX, LQT_METACALLER);')
	print_meta('\tlqtSlotAcceptor_'..module_name..' = new LqtSlotAcceptor(L);')
	print_meta('\treturn 0;\n}')
	if fmeta then fmeta:close() end
	return classes
end

local fix_methods_wrappers = function(classes)
	for c in pairs(classes) do
		c.shell = (not c.abstract) and c.public_destr
		c.shell = c.shell and (next(c.virtuals)~=nil)
		for _, constr in ipairs(c.constructors) do
			if c.shell then
				local shellname = 'lqt_shell_'..string.gsub(c.xarg.fullname, '::', '_LQT_')
				constr.calling_line = 'new '..shellname..'(L'
				if #(constr.arguments)>0 then constr.calling_line = constr.calling_line .. ', ' end
			else
				local shellname = c.xarg.fullname
				constr.calling_line = 'new '..shellname..'('
			end
			for i=1,#(constr.arguments) do
				constr.calling_line = constr.calling_line .. (i==1 and '' or ', ') .. 'arg' .. i
			end
			constr.calling_line = '*('..constr.calling_line .. '))'
			constr.xarg.static = '1'
			constr.return_type = constr.xarg.type_base..'&'
		end
		if c.destructor then
			c.destructor.return_type = nil
		end
	end
	return classes
end

local print_enum_tables = function(enums)
	for e in pairs(enums) do
		local table = 'static lqt_Enum lqt_enum'..e.xarg.id..'[] = {\n'
		--io.stderr:write(e.xarg.fullname, '\t', #e.values, '\n')
		for _,v in pairs(e.values) do
			table = table .. '  { "' .. v.xarg.name
				.. '", static_cast<int>('..v.xarg.fullname..') },\n'
		end
		table = table .. '  { 0, 0 }\n'
		table = table .. '};\n'
		e.enum_table = table
		print_enum(table)
	end
	return enums
end
local print_enum_creator = function(enums, n)
	local out = 'static lqt_Enumlist lqt_enum_list[] = {\n'
	for e in pairs(enums) do
		out = out..'  { lqt_enum'..e.xarg.id..', "'..string.gsub(e.xarg.fullname, "::", ".")..'" },\n'
	end
	out = out..'  { 0, 0 },\n};\n'
	out = out .. 'void lqt_create_enums_'..n..' (lua_State *L) {\n'
	out = out .. '  lqtL_createenumlist(L, lqt_enum_list);  return;\n}\n'
	print_enum(out)
	return enums
end

local copy_signals = function(functions)
	local ret = {}
	for f in pairs(functions) do
		if f.xarg.signal=='1' then
			ret[f] = 1
		end
	end
	return ret
end

local slots_for_signals = function(signals, types)
	local ret = {}
	for sig in pairs(signals) do
		local args, comma = '(', ''
		for i, a in ipairs(sig.arguments) do
			args = args .. comma .. a.xarg.type_name .. ' arg'..i
			comma = ', '
		end
		args = args .. ')'
		local pushlines, stack = make_pushlines(sig.arguments, types)
		if not ret['void __slot '..args] and pushlines then
			ret['void __slot '..args] = 'void LqtSlotAcceptor::__slot '..args..[[ {
  //int oldtop = lua_gettop(L);
  //lua_getfield(L, -1, "__slot]]..string.gsub(args, ' arg.', '')..[[");
  //if (lua_isnil(L, -1)) {
    //lua_pop(L, 1);
    //lua_getfield(L, -1, "__slot");
  //}
  //if (!lua_isfunction(L, -1)) {
    //lua_settop(L, oldtop);
    //return;
  //}
  lua_pushvalue(L, -2);
]] .. pushlines .. [[
  if (lqtL_pcall(L, ]]..stack..[[+1, 0, 0)) {
    //lua_error(L);
    qDebug() << lua_tostring(L, -1);
    lua_pop(L, 1);
  }
  //lua_settop(L, oldtop);
}
]]
		end
	end
	return ret
end

local print_slots = function(s)
	print_slot_h'class LqtSlotAcceptor : public QObject {'
	print_slot_h'  Q_OBJECT'
	print_slot_h'  lua_State *L;'
	print_slot_h'  public:'
	print_slot_h('  LqtSlotAcceptor(lua_State *l, QObject *p=NULL) : QObject(p), L(l) { setObjectName("'..module_name..'"); lqtL_register(L, this); }')
	print_slot_h'  virtual ~LqtSlotAcceptor() { lqtL_unregister(L, this); }'
	print_slot_h'  public slots:'
	for p, b in pairs(s) do
		print_slot_h('  '..p..';')
	end
	print_slot_h'};\n'
	print_slot_h('\nextern LqtSlotAcceptor *lqtSlotAcceptor_'..module_name..';')
	for p, b in pairs(s) do
		print_slot_c(b)
	end
	print_slot_c('\nLqtSlotAcceptor *lqtSlotAcceptor_'..module_name..';')
end


--------------------------------------------------------------------------------------

local typesystem = dofile(path..'types.lua')
do
	local ts = {}
	for i, ft in ipairs(typefiles) do
		ts = assert(loadfile(ft))(ts)
	end
	setmetatable(typesystem, {
		__newindex = function(t, k, v)
			--debug('added type', k)
			ts[k] = v
		end,
		__index = function(t, k)
			local ret = ts[k]
			--if not ret then debug("unknown type:", tostring(k), ret) end
			return ret
		end,
	})
end

fix_arguments(idindex) -- fixes default arguments if they are context-relative
local functions = copy_functions(idindex) -- picks functions and fixes label
local functions = fix_functions(functions) -- fixes name and fullname and fills arguments

local enums = copy_enums(idindex) -- picks enums if public
local enums = fill_enums(enums) -- fills field "values"

local classes = copy_classes(idindex) -- picks classes if not private and not blacklisted
local classes = fill_virtuals(classes) -- does that, destructor ("~") excluded
local classes = distinguish_methods(classes) -- does that
local classes = fill_public_destr(classes) -- does that: checks if destructor is public
local classes = fill_copy_constructor(classes) -- does that: checks if copy contructor is public or protected
local classes = fix_methods_wrappers(classes)
local classes = get_qobjects(classes)

for _, f in ipairs(filterfiles) do
	classes, enums = loadfile(f)(classes, enums)
end

local enums = fill_typesystem_with_enums(enums, typesystem) -- does that
local classes = fill_typesystem_with_classes(classes, typesystem)

local functions = fill_wrappers(functions, typesystem)
local classes = fill_virtual_overloads(classes, typesystem) -- does that
local classes = fill_shell_classes(classes, typesystem) -- does that

local signals = copy_signals(functions)
local slots = slots_for_signals(signals, typesystem)


------------- BEGIN OUTPUT



print_enum(output_includes)
print_slot_h('#ifndef LQT_SLOT_'..module_name)
print_slot_h('#define LQT_SLOT_'..module_name)
print_slot_h(output_includes)
print_slot_c('#include "'..module_name..'_slot.hpp'..'"\n\n')


local classes = print_shell_classes(classes) -- does that
local classes = print_virtual_overloads(classes, typesystem) -- does that
local enums = print_enum_tables(enums) -- does that
local enums = print_enum_creator(enums, module_name) -- does that + print enum list
local classes = print_wrappers(classes) -- just compiles metatable list
local classes = print_metatables(classes) -- just collects the wrappers + generates dispatchers
local classes = print_class_list(classes) -- does that + prints everything related to class

local slots = print_slots(slots)

print_slot_h('#endif')

--print_openmodule(module_name) -- does that

