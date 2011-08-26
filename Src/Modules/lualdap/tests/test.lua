#!/usr/local/bin/lua5.1
---------------------------------------------------------------------
-- LuaLDAP test file.
-- This test will create a copy of an existing entry on the
-- directory to work on.  This new entry will be modified,
-- renamed and deleted at the end.
--
-- See Copyright Notice in license.html
-- $Id: test.lua,v 1.15 2006-07-24 01:36:51 tomas Exp $
---------------------------------------------------------------------

--
DN_PAT = "^([^,=]+)%=([^,]+)%,?(.*)$"

---------------------------------------------------------------------
-- Print attributes.
---------------------------------------------------------------------
function print_attrs (dn, attrs)
	if not dn then
		io.write ("nil\n")
		return
	end
	io.write (string.format ("\t[%s]\n", dn))
	for name, values in pairs (attrs) do
		io.write ("["..name.."] : ")
		local tv = type (values)
		if tv == "string" then
			io.write (values)
		elseif tv == "table" then
			local n = table.getn (values)
			for i = 1, n-1 do
				io.write (values[i]..",")
			end
			io.write (values[n])
		end
		io.write ("\n")
	end
end

---------------------------------------------------------------------
-- clone a table.
---------------------------------------------------------------------
function clone (tab)
	local new = {}
	for i, v in pairs (tab) do
		new[i] = v
	end
	return new
end


---------------------------------------------------------------------
-- checks for a value and throw an error if it is not the expected.
---------------------------------------------------------------------
function assert2 (expected, value, msg)
	if not msg then
		msg = ''
	else
		msg = tostring(msg)..'\n'
	end
	local ret = assert (value == expected,
		msg.."wrong value (["..tostring(value).."] instead of "..
		tostring(expected)..")")
	io.write('.')
	return ret
end

---------------------------------------------------------------------
-- object test.
---------------------------------------------------------------------
function test_object (obj, objmethods)
	-- checking object type.
	assert2 ("userdata", type(obj), "incorrect object type")
	-- trying to get metatable.
	assert2 ("LuaLDAP: you're not allowed to get this metatable",
		getmetatable(obj), "error permitting access to object's metatable")
	-- trying to set metatable.
	assert2 (false, pcall (setmetatable, ENV, {}))
	-- checking existence of object's methods.
	for i = 1, table.getn (objmethods) do
		local method = obj[objmethods[i]]
		assert2 ("function", type(method))
		assert2 (false, pcall (method), "no 'self' parameter accepted")
	end
	return obj
end

CONN_OK = function (obj, err)
	if obj == nil then
		error (err, 2)
	end
	return test_object (obj, { "close", "add", "compare", "delete", "modify", "rename", "search", })
end

---------------------------------------------------------------------
-- basic checking test.
---------------------------------------------------------------------
function basic_test ()
	local ld = CONN_OK (lualdap.open_simple (HOSTNAME, WHO, PASSWORD))
	assert2 (1, ld:close(), "couldn't close connection")
	-- trying to close without a connection.
	assert2 (false, pcall (ld.close))
	-- trying to close an invalid connection.
	assert2 (false, pcall (ld.close, io.output()))
	-- trying to use a closed connection.
	local _,_,rdn_name,rdn_value = string.find (BASE, DN_PAT)
	assert2 (false, pcall (ld.compare, ld, BASE, rdn_name, rdn_value),
		"permitting the use of a closed connection")
	-- it is ok to close a closed object, but nil is returned instead of 1.
	assert2 (nil, ld:close())
	-- trying to connect to an invalid host.
	assert2 (nil, lualdap.open_simple ("unknown-server"), "this should be an error")
	-- reopen the connection.
	-- first, try using TLS
	local ok = lualdap.open_simple (HOSTNAME, WHO, PASSWORD, true)
	if not ok then
		-- second, try without TLS
		io.write ("\nWarning!  Couldn't connect with TLS.  Trying again without it.")
		ok = lualdap.open_simple (HOSTNAME, WHO, PASSWORD, false)
	end
	LD = CONN_OK (ok)
	CLOSED_LD = ld
	collectgarbage()
end


---------------------------------------------------------------------
-- checks return value which should be a function AND also its return value.
---------------------------------------------------------------------
function check_future (ret, method, ...)
	local ok, f = pcall (method, unpack (arg))
	assert (ok, f)
	assert2 ("function", type(f))
	assert2 (ret, f())
	io.write('.')
end


---------------------------------------------------------------------
-- checking compare operation.
---------------------------------------------------------------------
function compare_test ()
	local _,_,rdn_name,rdn_value = string.find (BASE, DN_PAT)
	assert (type(rdn_name) == "string", "could not extract RDN name")
	assert (type(rdn_value) == "string", "could not extract RDN value")
	-- comparing against the correct value.
	check_future (true, LD.compare, LD, BASE, rdn_name, rdn_value)
	-- comparing against a wrong value.
	check_future (false, LD.compare, LD, BASE, rdn_name, rdn_value..'_')
	-- comparing against an incorrect attribute name.
	check_future (nil, LD.compare, LD, BASE, rdn_name..'x', rdn_value)
	-- comparing on a wrong base.
	check_future (nil, LD.compare, LD, 'qwerty', rdn_name, rdn_value)
	-- comparing with a closed connection.
	assert2 (false, pcall (LD.compare, CLOSED_LD, BASE, rdn_name, rdn_value))
	-- comparing with an invalid userdata.
	assert2 (false, pcall (LD.compare, io.output(), BASE, rdn_name, rdn_value))
end


---------------------------------------------------------------------
-- checking basic search operation.
---------------------------------------------------------------------
function search_test_1 ()
	local _,_,rdn = string.find (WHO, "^([^,]+)%,.*$")
	local iter = LD:search {
		base = BASE,
		scope = "onelevel",
		sizelimit = 1,
		filter = "("..rdn..")",
	}
	assert2 ("function", type(iter))
	collectgarbage()
	CONN_OK (LD)
	local dn, entry = iter ()
	assert2 ("string", type(dn))
	assert2 ("table", type(entry))
	collectgarbage()
	assert2 ("function", type(iter))
	CONN_OK (LD)

	DN, ENTRY = LD:search {
		base = BASE,
		scope = "onelevel",
		sizelimit = 1,
		filter = "("..rdn..")",
	}()
	collectgarbage()
	assert2 ("string", type(DN))
	assert2 ("table", type(ENTRY))
end


---------------------------------------------------------------------
-- checking add operation.
---------------------------------------------------------------------
function add_test ()
	-- clone an entry.
	NEW = clone (ENTRY)
	local _,_,rdn_name, rdn_value, parent_dn = string.find (DN, DN_PAT)
	NEW[rdn_name] = rdn_value.."_copy"
	NEW_DN = string.format ("%s=%s,%s", rdn_name, NEW[rdn_name], parent_dn)
	-- trying to insert an entry with a wrong connection.
	assert2 (false, pcall (LD.add, CLOSED_LD, NEW_DN, NEW))
	-- trying to insert an entry with an invalid connection.
	assert2 (false, pcall (LD.add, io.output(), NEW_DN, NEW))
	-- trying to insert an entry with a wrong DN.
	local wrong_dn = string.format ("%s_x=%s,%s", rdn_name, NEW_DN, parent_dn)
	--assert2 (nil, LD:add (wrong_dn, NEW))
	check_future (nil, LD.add, LD, wrong_dn, NEW)
	-- trying to insert the clone on the LDAP data base.
	check_future (true, LD.add, LD, NEW_DN, NEW)
	-- trying to reinsert the clone entry on the directory.
	check_future (nil, LD.add, LD, NEW_DN, NEW)
end


---------------------------------------------------------------------
-- checking modify operation.
---------------------------------------------------------------------
function modify_test ()
	-- modifying without connection.
	assert2 (false, pcall (LD.modify, nil, NEW_DN, {}))
	-- modifying with a closed connection.
	assert2 (false, pcall (LD.modify, CLOSED_LD, NEW_DN, {}))
	-- modifying with an invalid userdata.
	assert2 (false, pcall (LD.modify, io.output(), NEW_DN, {}))
	-- checking invalid DN.
	assert2 (false, pcall (LD.modify, LD, {}))
	-- no modification to apply.
	check_future (true, LD.modify, LD, NEW_DN)
	-- forgotten operation on modifications table.
	local a_attr, a_value = next (ENTRY)
	assert2 (false, pcall (LD.modify, LD, NEW_DN, { [a_attr] = "abc"}))
	-- modifying an unknown entry.
	local _,_, rdn_name, rdn_value, parent_dn = string.find (NEW_DN, DN_PAT)
	local new_rdn = rdn_name..'='..rdn_value..'_'
	local new_dn = string.format ("%s,%s", new_rdn, parent_dn)
	check_future (nil, LD.modify, LD, new_dn)
	-- trying to create an undefined attribute.
	check_future (nil, LD.modify, LD, NEW_DN, {'+', unknown_attribute = 'a'})
end


---------------------------------------------------------------------
function count (tab)
	local counter = 0
	for dn, entry in LD:search (tab) do
		counter = counter + 1
	end
	return counter
end


---------------------------------------------------------------------
-- checking advanced search operation.
---------------------------------------------------------------------
function search_test_2 ()
	local _,_,rdn = string.find (WHO, "^([^,]+)%,.*$")
	local iter = LD:search {
		base = BASE,
		scope = "onelevel",
		sizelimit = 1,
		filter = "("..rdn..")",
	}
	assert2 ("function", type(iter))
	collectgarbage ()
	assert2 ("function", type(iter))
	local dn, entry = iter ()
	assert2 ("string", type(dn))
	assert2 ("table", type(entry))
	collectgarbage ()
	assert2 ("function", type(iter))
	iter = nil
	collectgarbage ()

	-- checking no search specification.
	assert2 (false, pcall (LD.search, LD))
	-- checking invalid scope.
	assert2 (false, pcall (LD.search, LD, { scope = 'BASE', base = BASE, }))
	-- checking invalid base.
	check_future (nil, LD.search, LD, { base = "invalid", scope = "base", })
	-- checking filter.
	local _,_, rdn_name, rdn_value, parent_dn = string.find (NEW_DN, DN_PAT)
	local filter = string.format ("(%s=%s)", rdn_name, rdn_value)
	assert (count { base = BASE, scope = "subtree", filter = filter, } == 1)
	-- checking sizelimit.
	assert (count { base = BASE, scope = "subtree", sizelimit = 1, } == 1)
	-- checking attrsonly parameter.
	for dn, entry in LD:search { base = BASE, scope = "subtree", attrsonly = true, } do
		for attr, value in pairs (entry) do
			assert (value == true, "attrsonly failed")
		end
	end
	-- checking reuse of search object.
	local iter = assert (LD:search { base = BASE, scope = "base", })
	assert (type(iter) == "function")
	local dn, e1 = iter()
	assert (type(dn) == "string")
	assert (type(e1) == "table")
	dn, e1 = iter()
	assert (type(dn) == "nil")
	assert (type(e1) == "nil")
	assert2 (false, pcall (iter))
	iter = nil
	-- checking collecting search objects.
	local dn, entry = LD:search { base = BASE, scope = "base" }()
	collectgarbage()
end


---------------------------------------------------------------------
-- checking rename operation.
---------------------------------------------------------------------
function rename_test ()
	local _,_, rdn_name, rdn_value, parent_dn = string.find (NEW_DN, DN_PAT)
	local new_rdn = rdn_name..'='..rdn_value..'_'
	local new_dn = string.format ("%s,%s", new_rdn, parent_dn)
	-- trying to rename with no parent.
	check_future (true, LD.rename, LD, NEW_DN, new_rdn, nil)
	-- trying to rename an invalid dn.
	check_future (nil, LD.rename, LD, NEW_DN, new_rdn, nil)
	-- trying to rename with the same parent.
	check_future (true, LD.rename, LD, new_dn, rdn_name..'='..rdn_value, parent_dn)
	-- trying to rename to an inexistent parent.
	check_future (nil, LD.rename, LD, NEW_DN, new_rdn, new_dn)
	-- mal-formed DN.
	assert2 (false, pcall (LD.rename, LD, ""))
	-- trying to rename with a closed connection.
	assert2 (false, pcall (LD.rename, CLOSED_LD, NEW_DN, new_rdn, nil))
	-- trying to rename with an invalid connection.
	assert2 (false, pcall (LD.rename, io.output(), NEW_DN, new_rdn, nil))
end


---------------------------------------------------------------------
-- checking delete operation.
---------------------------------------------------------------------
function delete_test ()
	-- trying to delete with a closed connection.
	assert2 (false, pcall (LD.delete, CLOSED_LD, NEW_DN))
	-- trying to delete with an invalid connection.
	assert2 (false, pcall (LD.delete, io.output(), NEW_DN))
	-- trying to delete new entry.
	check_future (true, LD.delete, LD, NEW_DN)
	-- trying to delete an already deleted entry.
	check_future (nil, LD.delete, LD, NEW_DN)
	-- mal-formed DN.
	check_future (nil, LD.delete, LD, "")
	-- no DN.
	assert2 (false, pcall (LD.delete, LD))
end


---------------------------------------------------------------------
-- checking close operation.
---------------------------------------------------------------------
function close_test ()
	assert (LD:close () == 1, "couldn't close connection")
end


---------------------------------------------------------------------
tests = {
	{ "basic checking", basic_test },
	{ "checking compare operation", compare_test },
	{ "checking basic search operation", search_test_1 },
	{ "checking add operation", add_test },
	{ "checking modify operation", modify_test },
	{ "checking advanced search operation", search_test_2 },
	{ "checking rename operation", rename_test },
	{ "checking delete operation", delete_test },
	{ "closing everything", close_test },
}

---------------------------------------------------------------------
-- Main
---------------------------------------------------------------------

if table.getn(arg) < 1 then
	print (string.format ("Usage %s host[:port] base [who [password]]", arg[0]))
	os.exit()
end

HOSTNAME = arg[1]
BASE = arg[2]
WHO = arg[3]
PASSWORD = arg[4]

require"lualdap"
assert (type(lualdap)=="table", "couldn't load LDAP library")

for i = 1, table.getn (tests) do
	local t = tests[i]
	io.write (t[1].." ...")
	t[2] ()
	io.write (" OK !\n")
end
