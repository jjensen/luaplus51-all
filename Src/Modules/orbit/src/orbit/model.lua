module("orbit.model", package.seeall)

model_methods = {}

dao_methods = {}

local type_names = {}

local function log_query(sql)
  io.stderr:write("[orbit.model] " .. sql .. "\n")
end

function type_names.sqlite3(t)
  return string.lower(string.match(t, "(%a+)"))
end

function type_names.mysql(t)
  if t == "number(1)" then
    return "boolean"
  else
    return string.lower(string.match(t, "(%a+)"))
  end
end

local convert = {}

function convert.integer(v)
  return tonumber(v)
end

function convert.int(v)
  return tonumber(v)
end

function convert.number(v)
  return tonumber(v)
end

function convert.varchar(v)
  return tostring(v)
end

function convert.string(v)
  return tostring(v)
end

function convert.text(v)
  return tostring(v)
end

function convert.boolean(v, driver)
  if driver == "sqlite3" then
    return v == "t"
  elseif driver == "mysql" then
    return tonumber(v) == 1
  else
    error("driver not supported")
  end
end

function convert.binary(v)
  return convert.text(v)
end

function convert.datetime(v)
  local year, month, day, hour, min, sec = 
    string.match(v, "(%d+)%-(%d+)%-(%d+) (%d+):(%d+):(%d+)")
  return os.time({ year = tonumber(year), month = tonumber(month),
		   day = tonumber(day), hour = tonumber(hour),
		   min = tonumber(min), sec = tonumber(sec) })
end

local function convert_types(row, meta, driver)
  for k, v in pairs(row) do
    if meta[k] then
      local conv = convert[meta[k].type]
      if conv then
	row[k] = conv(v, driver)
      else
	error("no conversion for type " .. meta[k].type)
      end
    end
  end
end

local escape = {}

function escape.integer(v)
  return tostring(v)
end

function escape.int(v)
  return tostring(v)
end

function escape.number(v)
  return escape.integer(v)
end

function escape.varchar(v, driver, conn)
  return "'" .. conn:escape(v) .. "'"
end

function escape.string(v, driver, conn)
  return escape.varchar(v, driver, conn)
end

function escape.text(v, driver, conn)
  return "'" .. conn:escape(v) .. "'"
end

function escape.datetime(v)
  return "'" .. os.date("%Y-%m-%d %H:%M:%S", v) .. "'"
end

function escape.boolean(v, driver)
  if v then
    if driver == "sqlite3" then return "'t'" else return tostring(v) end
  else
    if driver == "sqlite3" then return "'f'" else return tostring(v) end
  end
end

function escape.binary(v, driver, conn)
  return escape.text(v, driver, conn)
end

local function escape_values(row)
  local row_escaped = {}
  for i, m in ipairs(row.meta) do
    if row[m.name] == nil then
      row_escaped[m.name] = "NULL" 
    else
      local esc = escape[m.type]
      if esc then
	row_escaped[m.name] = esc(row[m.name], row.driver, row.model.conn)
      else
	error("no escape function for type " .. m.type)
      end
    end
  end
  return row_escaped
end

local function fetch_row(dao, sql)
  local cursor, err = dao.model.conn:execute(sql)
  if not cursor then error(err) end
  local row = cursor:fetch({}, "a")
  cursor:close()
  if row then
    convert_types(row, dao.meta, dao.driver)
    setmetatable(row, { __index = dao })
  end
  return row
end

local function fetch_rows(dao, sql, count)
  local rows = {}
  local cursor, err = dao.model.conn:execute(sql)
  if not cursor then error(err) end
  local row, fetched = cursor:fetch({}, "a"), 1
  while row and (not count or fetched <= count) do
    convert_types(row, dao.meta, dao.driver)
    setmetatable(row, { __index = dao })
    rows[#rows + 1] = row
    row, fetched = cursor:fetch({}, "a"), fetched + 1
  end
  cursor:close()
  return rows
end

local function parse_condition(dao, condition, args)
  condition = string.gsub(condition, "_and_", "|")
  local pairs = {}
  for field in string.gmatch(condition, "[%w_]+") do
    local i = #pairs + 1
    local value
    if args[i] == nil then
      pairs[i] = field .. " is null"
    elseif type(args[i]) == "table" then
      local values = {}
      for _, value in ipairs(args[i]) do
	values[#values + 1] = escape[dao.meta[field].type](value, dao.driver, dao.model.conn)
      end
      pairs[i] = field .. " IN (" .. table.concat(values,", ") .. ")"
    else
      value = escape[dao.meta[field].type](args[i], dao.driver, dao.model.conn)
      pairs[i] = field .. " = " .. value
    end
  end
  return pairs
end

local function build_inject(project, inject, dao)
  local fields = {}
  if project then
     for i, field in ipairs(project) do
	fields[i] = dao.table_name .. "." .. field .. " as " .. field
     end
  else
     for i, field in ipairs(dao.meta) do
	fields[i] = dao.table_name .. "." .. field.name .. " as " .. field.name
     end
  end
  local inject_fields = {}
  local model = inject.model
  for _, field in ipairs(inject.fields) do
    inject_fields[model.name .. "_" .. field] =
      model.meta[field]
    fields[#fields + 1] = model.table_name .. "." .. field .. " as " ..
      model.name .. "_" .. field
  end
  setmetatable(dao.meta, { __index = inject_fields })
  return table.concat(fields, ", "), dao.table_name .. ", " .. 
    model.table_name,  model.name .. "_id = " .. model.table_name .. ".id"
end

local function build_query_by(dao, condition, args)
  local pairs = parse_condition(dao, condition, args)
  local order = ""
  local field_list, table_list, select, limit
  if args.distinct then select = "select distinct " else select = "select " end
  if tonumber(args.count) then limit = " limit " .. tonumber(args.count) else limit = "" end
  if args.order then order = " order by " .. args.order end
  if args.inject then
    field_list, table_list, pairs[#pairs + 1] = build_inject(args.fields, args.inject,
      dao)
  else
    if args.fields then
       field_list = table.concat(args.fields, ", ")
    else
       field_list = "*"
    end
    table_list = dao.table_name
  end
  local sql = select .. field_list .. " from " .. table_list ..
    " where " .. table.concat(pairs, " and ") .. order .. limit
  if dao.model.logging then log_query(sql) end
  return sql
end

local function find_by(dao, condition, args)
  return fetch_row(dao, build_query_by(dao, condition, args))
end

local function find_all_by(dao, condition, args)
  return fetch_rows(dao, build_query_by(dao, condition, args), args.count)
end

local function dao_index(dao, name)
  local m = dao_methods[name]
  if m then
    return m
  else
    local match = string.match(name, "^find_by_(.+)$")
    if match then
      return function (dao, args) return find_by(dao, match, args) end
    end
    local match = string.match(name, "^find_all_by_(.+)$")
    if match then
      return function (dao, args) return find_all_by(dao, match, args) end
    end
    return nil
  end
end

function model_methods:new(name, dao)
  dao = dao or {}
  dao.model, dao.name, dao.table_name, dao.meta, dao.driver = self, name, 
    self.table_prefix .. name, {}, self.driver
  setmetatable(dao, { __index = dao_index })
  local sql = "select * from " .. dao.table_name .. " limit 0"
  if self.logging then log_query(sql) end
  local cursor, err = self.conn:execute(sql)
  if not cursor then error(err) end
  local names, types = cursor:getcolnames(), cursor:getcoltypes()
  cursor:close()
  for i = 1, #names do
    local colinfo = { name = names[i],
    type = type_names[self.driver](types[i]) }
    dao.meta[i] = colinfo
    dao.meta[colinfo.name] = colinfo
  end
  return dao
end

function recycle(fresh_conn, timeout)
  local created_at = os.time()
  local conn = fresh_conn()
  timeout = timeout or 20000
  return setmetatable({}, { __index = function (tab, meth)
					 tab[meth] = function (tab, ...)
							if created_at + timeout < os.time() then
							   created_at = os.time()
							   pcall(conn.close, conn)
							   conn = fresh_conn()
							end
							return conn[meth](conn, ...)
						     end
					 return tab[meth]
				      end
			 })
end

function new(table_prefix, conn, driver, logging)
  driver = driver or "sqlite3"
  local app_model = { table_prefix = table_prefix or "", conn = conn, driver = driver or "sqlite3", logging = logging, models = {} }
  setmetatable(app_model, { __index = model_methods })
  return app_model
end

function dao_methods.find(dao, id, inject)
  if not type(id) == "number" then
    error("find error: id must be a number")
  end
  if dao.logging then log_query(sql) end
  local sql = "select * from " .. dao.table_name ..
    " where id=" .. id
  return fetch_row(dao, sql)
end

local function build_query(dao, condition, args)
  local i = 0
  args = args or {}
  condition = condition or ""
  if type(condition) == "table" then
    args = condition
    condition = ""
  end
  if condition ~= "" then
    condition = " where " ..
      string.gsub(condition, "([%w_]+)%s*([%a<>=]+)%s*%?",
		  function (field, op)
		    i = i + 1
		    if not args[i] then
		      return "id=id"
		    elseif type(args[i]) == "table" then
		      local values = {}
		      for j, value in ipairs(args[i]) do
			values[#values + 1] = field .. " " .. op .. " " ..
		          escape[dao.meta[field].type](value, dao.driver, dao.model.conn)
                      end
		      return "(" .. table.concat(values, " or ") .. ")"
                    else
		      return field .. " " .. op .. " " ..
		        escape[dao.meta[field].type](args[i], dao.driver, dao.model.conn)
                    end
		  end)
  end
  local order = ""
  if args.order then order = " order by " .. args.order end
  local field_list, table_list, select, limit
  if args.distinct then select = "select distinct " else select = "select " end
  if tonumber(args.count) then limit = " limit " .. tonumber(args.count) else limit = "" end
  if args.inject then
    local inject_condition
    field_list, table_list, inject_condition = build_inject(args.fields, args.inject,
      dao)
    if condition == "" then
      condition = " where " .. inject_condition
    else
      condition = condition .. " and " .. inject_condition
    end
  else
    if args.fields then
       field_list = table.concat(args.fields, ", ")
    else
       field_list = "*"
    end
    table_list = dao.table_name
  end
  local sql = select .. field_list .. " from " .. table_list .. 
    condition .. order .. limit
  if dao.model.logging then log_query(sql) end
  return sql
end

function dao_methods.find_first(dao, condition, args)
  return fetch_row(dao, build_query(dao, condition, args))
end

function dao_methods.find_all(dao, condition, args)
  return fetch_rows(dao, build_query(dao, condition, args), 
		    (args and args.count) or (condition and condition.count))
end

function dao_methods.new(dao, row)
  row = row or {}
  setmetatable(row, { __index = dao })
  return row
end

local function update(row)
  local row_escaped = escape_values(row)
  local updates = {}
  if row.meta["updated_at"] then
    local now = os.time()
    row.updated_at = now
    row_escaped.updated_at = escape.datetime(now, row.driver)
  end
  for k, v in pairs(row_escaped) do
    table.insert(updates, k .. "=" .. v)
  end
  local sql = "update " .. row.table_name .. " set " ..
    table.concat(updates, ", ") .. " where id = " .. row.id
  if row.model.logging then log_query(sql) end
  local ok, err = row.model.conn:execute(sql)
  if not ok then error(err) end
end

local function insert(row)
  local row_escaped = escape_values(row)
  local now = os.time()
  if row.meta["created_at"] then
    row.created_at = row.created_at or now
    row_escaped.created_at = escape.datetime(now, row.driver)
  end
  if row.meta["updated_at"] then
    row.updated_at = row.updated_at or now
    row_escaped.updated_at = escape.datetime(now, row.driver)
  end
  local columns, values = {}, {}
  for k, v in pairs(row_escaped) do
    table.insert(columns, k)
    table.insert(values, v)
  end
  local sql = "insert into " .. row.table_name ..
    " (" .. table.concat(columns, ", ") .. ") values (" ..
    table.concat(values, ", ") .. ")"
  if row.model.logging then log_query(sql) end
  local ok, err = row.model.conn:execute(sql)
  if ok then 
    row.id = row.id or row.model.conn:getlastautoid()
  else 
    error(err)
  end
end

function dao_methods.save(row, force_insert)
  if row.id and (not force_insert) then
    update(row)
  else
    insert(row)
  end
end

function dao_methods.delete(row)
  if row.id then
    local sql = "delete from " .. row.table_name .. " where id = " .. row.id
    if row.model.logging then log_query(sql) end
    local ok, err = row.model.conn:execute(sql)    
    if ok then row.id = nil else error(err) end
  end
end
