module("toycms", package.seeall)

function plugins.date(web)
  return {
    today_day = tonumber(os.date("%d", os.time())),
    today_month_name = month_names[tonumber(os.date("%m", os.time()))],
    today_year = tonumber(os.date("%Y", os.time()))
  }
end

function plugins.archive(web)
  return {
    month_list = function (arg, has_block)
      if arg and arg.include_tags then
        local sections = 
	  models.section:find_all("tag like ?", { arg.include_tags })
	local section_ids = {}
	for _, section in ipairs(sections) do
	  section_ids[#section_ids + 1] = section.id
        end
        local months = models.post:find_months(section_ids)
	local out, template
	if not has_block then
	  out = {}
	  template = load_template(arg.template or 
				   "month_list.html")
	end
	for _, month in ipairs(months) do
	  local env = new_template_env(web)
	  env.month = month.month
	  env.year = month.year
	  env.month_padded = string.format("%.2i", month.month)
	  env.month_name = month_names[month.month]
	  env.uri = web:link("/archive/" .. env.year .. "/" ..
			     env.month_padded)
	  if has_block then
	    cosmo.yield(env)
	  else
	    local tdata = template(env)
	    table.insert(out, tdata)
	  end
	end
	if not has_block then return table.concat(out, "\n") end
      else
        return ((not has_block) and "") or nil
      end
    end
  }
end

function plugins.section_list(web)
  return {
    section_list = function (arg, has_block)
      arg = arg or {}
      local template_name = arg.template
      if arg.include_tags then
        arg = { arg.include_tags }
        arg.condition = "tag like ?"
      end
      local out, template
      if not has_block then
	out = {}
	template = load_template(template_name or 
				 "section_list.html")
      end
      local sections = models.section:find_all(arg.condition, arg)
      for _, section in ipairs(sections) do
        web.input.section_id = section.id
        local env = new_section_env(web, section)
	if has_block then
	  cosmo.yield(env)
	else
	  local tdata = template(env)
	  table.insert(out, tdata)
	end
      end
      if not has_block then return table.concat(out, "\n") end
    end
  }
end

local function get_posts(web, condition, args, count, template)
  local posts =
    models.post:find_all(condition, args)
  local cur_date
  local out
  if template then out = {} end
  for i, post in ipairs(posts) do
    if count and (i > count) then break end
    local env = new_post_env(web, post)
    env.if_new_date = cosmo.cond(cur_date ~= env.date_string, env)
    if cur_date ~= env.date_string then
      cur_date = env.date_string
    end
    env.if_first = cosmo.cond(i == 1, env)
    env.if_not_first = cosmo.cond(i ~= 1, env)
    env.if_last = cosmo.cond(i == #posts, env)
    env.if_not_post = cosmo.cond(web.input.post_id ~= post.id, env)
    if template then
      local tdata = template(env)
      table.insert(out, tdata)
    else
      cosmo.yield(env)
    end
  end
  if template then return table.concat(out, "\n") end
end

function plugins.home(web)
  return {
    headlines = function (arg, has_block)
		  local template
		  if not has_block then 
		    template = load_template("home_short_info.html")
		  end
		  return get_posts(web, "in_home = ? and published = ?",
				   { order = "published_at desc", true, true },
				   nil, template)
		end
  }
end

function plugins.index_view(web)
  return {
    show_posts = function (arg, has_block)
      local section_ids = {}
      local template_file = (arg and arg.template) or "index_short_info.html"   
      if arg and arg.include_tags then
	local sections = models.section:find_by_tags{ arg.include_tags }
        for _, section in ipairs(sections) do
          section_ids[#section_ids + 1] = section.id
        end
      elseif web.input.section_id then
        section_ids[#section_ids + 1] = web.input.section_id
      end
      if #section_ids == 0 then return "" end
      local date_start, date_end
      if arg and arg.archive and web.input.month and web.input.year then
        date_start = os.time({ year = web.input.year, 
			    month = web.input.month, day = 1 })
        date_end = os.time({ year = web.input.year + 
			    math.floor(web.input.month / 12),
                            month = (web.input.month % 12) + 1,
                            day = 1 })
      end
      local template
      if not has_block then template = load_template(template_file) end
      return get_posts(web, "published = ? and section_id = ? and " ..
		       "published_at >= ? and published_at <= ?",
		     { order = "published_at desc", true, 
		       section_ids, date_start,
		       date_end },
		     (arg and arg.count), template)
    end
  }
end
