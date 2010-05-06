#include "lqt_qt.hpp"


int lqtL_qt_metacall (lua_State *L, QObject *self, QObject *acceptor,
        QMetaObject::Call call, const char *name,
        int index, void **args) {
    int callindex = 0, oldtop = 0;
    oldtop = lua_gettop(L);
    lqtL_pushudata(L, self, name); // (1)
    lua_getfield(L, -1, LQT_OBJSIGS); // (2)
    //qDebug() << lua_gettop(L) << luaL_typename(L, -1);
    lua_rawgeti(L, -1, index + 1); // (3)
    if (!lua_isstring(L, -1)) {
        lua_pop(L, 3); // (0)
        lua_settop(L, oldtop); // (0)
        QMetaObject::activate(self, self->metaObject(), index, args);
    } else {
        callindex = acceptor->metaObject()->indexOfSlot(lua_tostring(L, -1));
        // qDebug() << "Found slot" << name << lua_tostring(L,-1) << "on" << acceptor->objectName() << "with index" << callindex;
        lua_pop(L, 2); // (1)
        lua_getfield(L, -1, LQT_OBJSLOTS); // (2)
        lua_rawgeti(L, -1, index+1); // (3)
        lua_remove(L, -2); // (2)
        index = acceptor->qt_metacall(call, callindex, args);
    }
    return -1;
}


const char add_method_func[] =
"return function(qobj, signature, func)\n"
"	local qname = 'LuaObject('..tostring(qobj)..')'\n"
"	local stringdata = qobj['"LQT_OBJMETASTRING"']\n"
"	local data = qobj['"LQT_OBJMETADATA"']\n"
"	local slots = qobj['"LQT_OBJSLOTS"']\n"
"	local sigs = qobj['"LQT_OBJSIGS"']\n"
"	if stringdata==nil then\n"
"		--print'adding a slot!'\n"
"		--initialize\n"
"		stringdata = qname..'\\0'\n"
"		data = setmetatable({}, {__index=table})\n"
"		data:insert(1) -- revision\n"
"		data:insert(0) -- class name\n"
"		data:insert(0) -- class info (1)\n"
"		data:insert(0) -- class info (2)\n"
"		data:insert(0) -- number of methods\n"
"		data:insert(10) -- beginning of methods\n"
"		data:insert(0) -- number of properties\n"
"		data:insert(0) -- beginning of properties\n"
"		data:insert(0) -- number of enums/sets\n"
"		data:insert(0) -- beginning of enums/sets\n"
"		slots = setmetatable({}, {__index=table})\n"
"		sigs = setmetatable({}, {__index=table})\n"
"	end\n"
"	local name, args = string.match(signature, '^(.*)(%b())$')\n"
"	local arg_list = ''\n"
"	if args=='()' then\n"
"		arg_list=''\n"
"	else\n"
"		local argnum = select(2, string.gsub(args, '.+,', ','))+1\n"
"		for i = 1, argnum do\n"
"			if i>1 then arg_list=arg_list..', ' end\n"
"			arg_list = arg_list .. 'arg' .. i\n"
"		end\n"
"	end\n"
"	--print(arg_list, signature)\n"
"	local sig, params = #stringdata + #arg_list + 1, #stringdata -- , ty, tag, flags\n"
"	stringdata = stringdata .. arg_list .. '\\0' .. signature .. '\\0'\n"
"	data:insert(sig) -- print(sig, string.byte(stringdata, sig, sig+4), string.char(string.byte(stringdata, sig+1, sig+6)))\n"
"	data:insert(params) -- print(params, string.char(string.byte(stringdata, params+1, params+10)))\n"
"	data:insert(#stringdata-1) -- print(#stringdata-1, (string.byte(stringdata, #stringdata)))\n"
"	data:insert(#stringdata-1) -- print(#stringdata-1, (string.byte(stringdata, #stringdata)))\n"
"	if func then\n"
"		data:insert(0x0a)\n"
"		slots:insert(func)\n"
"		sigs:insert('__slot'..signature:match'%b()')\n"
"	else\n"
"		data:insert(0x05)\n"
"		slots:insert(false)\n"
"		sigs:insert(false)\n"
"	end\n"
"	data[5] = data[5] + 1\n"
"	qobj['"LQT_OBJMETASTRING"'] = stringdata\n"
"	qobj['"LQT_OBJMETADATA"'] = data\n"
"	qobj['"LQT_OBJSLOTS"'] = slots\n"
"	qobj['"LQT_OBJSIGS"'] = sigs\n"
"end\n";

void lqtL_pushaddmethod (lua_State *L) {
    luaL_dostring(L, add_method_func);
}

