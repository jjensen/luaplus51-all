module(..., package.seeall)

-- Loosely based on a PHP implementation.
local lom = require "lxp.lom"

local function xml_depth(vals)
	local valCount = #vals
	if valCount == 1  and  type(vals[1]) ~= 'table' then
		return vals[1]
	end

	local children = {}

	for i = 1, #vals do
		local val = vals[i]
		if type(val) == "table" then
			children[#children + 1] = val.tag
			local tagEntry = children[val.tag]
			if not tagEntry then
				tagEntry = {}
				children[val.tag] = tagEntry
			end

			entry = {}
			tagEntry[#tagEntry + 1] = entry

			entry['@'] = val.attr
			entry['#'] = xml_depth(val)
		else
			children[#children + 1] = val
		end
	end

	return children
end


function luaize(data)
	data = data:gsub('<%?xml.-%?>(.+)', "%1")
	data = '<root>' .. data .. '</root>'

	local vals, err = lom.parse(data)

    array = xml_depth(vals);

	return array
end


local srep = string.rep

local function xmlsave_recurse(indent, luaTable, xmlTable, entryOrderValue)
	local tabs = indent and srep('\t', indent) or ''
	local keys = {}
	local entryOrder
	if luaTable[1] then
		for _, key in ipairs(luaTable) do
			local whichIndex = keys[key]
			if not whichIndex then
				keys[key] = 0
				whichIndex = 0
			end
			whichIndex = whichIndex + 1
			keys[key] = whichIndex

			local section = luaTable[key]
			if not section then
				if not indent then
					-- Generally whitespace.
					xmlTable[#xmlTable + 1] = key
				end
			else
				local entry = section[whichIndex]
				if not entry then
					error('xmlsave: syntax bad')
				end

				xmlTable[#xmlTable + 1] = tabs .. '<' .. key

				local attributes = entry['@']
				if attributes then
					for _, attrKey in ipairs(attributes) do
						xmlTable[#xmlTable + 1] = ' ' .. attrKey .. '="' .. attributes[attrKey] .. '"'
					end
				end

				xmlTable[#xmlTable + 1] = '>'

				local elements = entry['#']
				if type(elements) == 'table' then
					if indent then
						xmlTable[#xmlTable + 1] = '\n'
					end
					xmlsave_recurse(indent and (indent + 1) or nil, elements, xmlTable)
				else
					xmlTable[#xmlTable + 1] = elements
				end

				if indent and type(elements) == 'table' then
					xmlTable[#xmlTable + 1] = tabs
				end
				xmlTable[#xmlTable + 1] = '</' .. key .. '>'
				if indent then
					xmlTable[#xmlTable + 1] = '\n'
				end
			end
		end
	else
		for key, value in pairs(luaTable) do
			if type(value) == 'table' then
				for _, entry in ipairs(value) do
					xmlTable[#xmlTable + 1] = tabs .. '<' .. key

					local attributes = entry['@']
					if attributes then
						for _, attrKey in ipairs(attributes) do
							xmlTable[#xmlTable + 1] = ' ' .. attrKey .. '="' .. attributes[attrKey] .. '"'
						end
					end

					xmlTable[#xmlTable + 1] = '>'

					local elements = entry['#']
					if type(elements) == 'table' then
						if indent then
							xmlTable[#xmlTable + 1] = '\n'
						end
						xmlsave_recurse(indent and (indent + 1) or nil, elements, xmlTable)
					else
						xmlTable[#xmlTable + 1] = elements
					end

					if indent and type(elements) == 'table' then
						xmlTable[#xmlTable + 1] = tabs
					end
					xmlTable[#xmlTable + 1] = '</' .. key .. '>'
					if indent then
						xmlTable[#xmlTable + 1] = '\n'
					end
				end
			end
		end
	end
end


function xmlize(outFilename, luaTable, indent)
	local xmlTable = {}
	xmlsave_recurse(indent, luaTable, xmlTable)
	local outText = table.concat(xmlTable)
	if outFilename == ':string' then
		return outText
	else
		local file = io.open(outFilename, "wt")
		file:write(table.concat(xmlTable))
		file:close()
	end
end

