
module("toycms", package.seeall)

-- Admin interface

function admin(web, section_id)
   if not check_user(web) then
      return web:redirect(web:link("/login", { link_to = web:link("/admin") }))
   else
      local params = {}
      if tonumber(section_id) then
	 params.section = models.section:find(tonumber(section_id))
      end
      return admin_layout(web, render_admin(web, params))
   end
end

toycms:dispatch_get(admin, "/admin", "/admin/(%d+)")

function login_get(web)
   return admin_layout(web, render_login(web, web.input))	    
end

function login_post(web)
   local login = web.input.login
   local password = web.input.password
   local user = models.user:find_by_login{ login }
   if web:empty_param("link_to") then
      web.input.link_to = web:link("/")
   end
   if user then
      if password == user.password then
	 web:set_cookie("user_id", user.id)
	 web:set_cookie("password", user.password)
	 return web:redirect(web.input.link_to)
      else
	 return web:redirect(web:link("/login", { login = login,
					 link_to = web.input.link_to,
					 not_match = "1" }))
      end
   else
      return web:redirect(web:link("/login", { login = login,
				      link_to = web.input.link_to,
				      not_found = "1" }))
   end
end

toycms:dispatch_get(login_get, "/login")
toycms:dispatch_post(login_post, "/login")

function add_user_get(web)
   if not check_user(web) then
      return web:redirect(web:link("/login", { link_to = web:link("/adduser") }))
   else
      return admin_layout(web, render_add_user(web, web.input))
   end
end

function add_user_post(web)
   if not check_user(web) then
      return web:redirect(web:link("/login", { link_to = web:link("/adduser") }))
   else
      local errors = {}
      if web:empty_param("login") then
	 errors.login = strings.blank_user
      end
      if web:empty_param("password1") then
	 errors.password = strings.blank_password
      end
      if web.input.password1 ~= web.input.password2 then
	 errors.password = strings.password_mismatch
      end
      if web:empty_param("name") then
	 errors.name = strings.blank_name
      end
      if not next(errors) then
	 local user = models.user:new()
	 user.login = web.input.login
	 user.password = web.input.password1
	 user.name = web.input.name
	 user:save()
	 return web:redirect(web:link("/admin"))
      else
	 for k, v in pairs(errors) do web.input["error_" .. k] = v end
	 return web:redirect(web:link("/adduser", web.input))
      end
   end
end

toycms:dispatch_get(add_user_get, "/adduser")
toycms:dispatch_post(add_user_post, "/adduser")

function edit_section_get(web, section_id)
   if not check_user(web) then
      return web:redirect(web:link("/login", { link_to = web:link("/editsection") }))
   else
      section_id = tonumber(section_id)
      if section_id then
	 web.input.section = models.section:find_by_id{ section_id }
      else
	 web.input.section = models.section:new()	
      end
      return admin_layout(web, render_edit_section(web,  web.input))
   end
end

function edit_section_post(web, section_id)
   if not check_user(web) then
      return web:redirect(web:link("/login", { link_to = web:link("/editsection") }))
   else
      section_id = tonumber(section_id)
      local errors = {}
      if web:empty_param("title") then
	 errors.title = strings.blank_title
      end
      if not next(errors) then
	 local section
	 if section_id then
	    section = models.section:find_by_id{ section_id }
	 else
	    section = models.section:new()
	 end
	 section.title = web.input.title
	 section.description = web.input.description
	 section.tag = web.input.tag
	 section:save()
	 cache:nuke()
	 return web:redirect(web:link("/editsection/" .. section.id))
      else
	 for k, v in pairs(errors) do web.input["error_" .. k] = v end
	 if section_id then
            return web:redirect(web:link("/editsection/" .. section_id, web.input))
	 else
            return web:redirect(web:link("/editsection", web.input))
	 end
      end
   end
end

toycms:dispatch_get(edit_section_get, "/editsection", "/editsection/(%d+)")
toycms:dispatch_post(edit_section_post, "/editsection", "/editsection/(%d+)")

function delete_section(web, section_id)
   if not check_user(web) then
      return web:redirect(web:link("/login", 
				   { link_to = web:link("/editsection/" .. section_id) }))
   else
      section_id = tonumber(section_id)
      local section = models.section:find(section_id)
      if section then
	 section:delete()
	 cache:nuke()
	 return web:redirect(web:link("/admin"))
      end
   end   
end

toycms:dispatch_post(delete_section, "/deletesection/(%d+)")

function delete_post(web, post_id)
   if not check_user(web) then
      return web:redirect(web:link("/login", 
				   { link_to = web:link("/editpost/" .. post_id) }))
   else
      post_id = tonumber(post_id)
      local post = models.post:find(post_id)
      if post then
	 post:delete()
	 cache:nuke()
	 return web:redirect(web:link("/admin"))
      end
   end   
end

toycms:dispatch_post(delete_post, "/deletepost/(%d+)")

function edit_post_get(web, post_id)
   if not check_user(web) then
      return web:redirect(web:link("/login", { link_to = web:link("/editpost") }))
   else
      post_id = tonumber(post_id)
      if post_id then
	 web.input.post = models.post:find_by_id{ post_id }
      else
	 web.input.post = models.post:new()	
      end
      web.input.sections = models.section:find_all()
      return admin_layout(web, render_edit_post(web,  web.input))
   end
end

function edit_post_post(web, post_id)
   if not check_user(web) then
      return web:redirect(web:link("/login", { link_to = web:link("/editpost") }))
   else
      post_id = tonumber(post_id)
      local errors = {}
      if web:empty_param("title") then
	 errors.title = strings.blank_title
      end
      if not next(errors) then
	 local post
	 if post_id then
	    post = models.post:find_by_id{ post_id }
	 else
	    post = models.post:new()
	 end
	 post.title = web.input.title
	 post.abstract = web.input.abstract
	 post.image = web.input.image
	 if web:empty_param("published_at") then
	    post.published_at = nil
	 else
	    local day, month, year, hour, minute =
	       string.match(web.input.published_at,
			    "(%d+)-(%d+)-(%d+) (%d+):(%d+)")
	    post.published_at = os.time({ day = day, month = month,
					   year = year, hour = hour,
					   min = minute })
	 end
	 post.body = web.input.body
	 post.section_id = tonumber(web.input.section_id)
	 post.published = (not web:empty_param("published"))
	 post.external_url = web.input.external_url
	 post.in_home = (not web:empty_param("in_home"))
	 post.user_id = check_user(web).id
	 post.comment_status = web.input.comment_status
	 post:save()
	 cache:nuke()
	 return web:redirect(web:link("/editpost/" .. post.id))
      else
	 for k, v in pairs(errors) do web.input["error_" .. k] = v end
	 if post_id then
            return web:redirect(web:link("/editpost/" .. post_id, web.input))
	 else
            return web:redirect(web:link("/editpost", web.input))
	 end
      end
   end
end

toycms:dispatch_get(edit_post_get, "/editpost", "/editpost/(%d+)")
toycms:dispatch_post(edit_post_post, "/editpost", "/editpost/(%d+)")

function manage_comments(web)
   if not check_user(web) then
      return web:redirect(web:link("/login", { link_to = web:link("/comments") }))
   else
      local params = {}
      local post_model = models.post
      params.for_mod = models.comment:find_all_by_approved{ false,
	 order = "created_at desc", inject = { model = post_model, 
	    fields = { "title" } } }
      params.approved = models.comment:find_all_by_approved{ true,
	 order = "created_at desc", inject = { model = post_model, 
	    fields = { "title" } } }
      return admin_layout(web, render_manage_comments(web,  params))
   end
end

toycms:dispatch_get(manage_comments, "/comments")

function approve_comment(web, id)
   if not check_user(web) then
      return web:redirect(web:link("/login", { link_to = web:link("/comments#" ..
								  id) }))
   else
      local comment = models.comment:find(tonumber(id))
      if comment then
	 comment.approved = true
	 comment:save()
	 local post = models.post:find(comment.post_id)
	 post.n_comments = (post.n_comments or 0) + 1
	 post:save()
	 cache:invalidate("/", "/xml", "/section/" .. post.section_id,
			  "/section/" .. post.section_id .. "/xml",
			  "/post/" .. post.id, "/post/" .. post.id .. "/xml",
			  "/archive/" .. 
			     string.format("%Y/%m", post.published_at))
	 return web:redirect(web:link("/comments#" .. id))
      end
   end
end

toycms:dispatch_post(approve_comment, "/comment/(%d+)/approve")

function delete_comment(web, id)
   if not check_user(web) then
      return web:redirect(web:link("/login", { link_to = web:link("/comments#" ..
								  id) }))
   else
      local comment = models.comment:find(tonumber(id))
      if comment then
	 if comment.approved then
  	    local post = models.post:find(comment.post_id)
	    post.n_comments = post.n_comments - 1
	    post:save()
	    cache:invalidate("/", "/xml", "/section/" .. post.section_id,
			     "/section/" .. post.section_id .. "/xml",
			     "/post/" .. post.id, "/post/" .. post.id .. "/xml",
			     "/archive/" .. 
				string.format("%Y/%m", post.published_at))
	 end
	 comment:delete()
	 return web:redirect(web:link("/comments"))
      end
   end
end

toycms:dispatch_post(delete_comment, "/comment/(%d+)/delete")

toycms:dispatch_static("/admin_style%.css")

local function error_message(msg)
  return "<span style = \"color: red\">" ..  msg .. "</span>"
end

function admin_layout(web, inner_html)
   return html{
      head{
	 title"ToyCMS Admin",
	 meta{ ["http-equiv"] = "Content-Type",
	    content = "text/html; charset=utf-8" },
	 link{ rel = 'stylesheet', type = 'text/css', 
	    href = web:static_link('/admin_style.css'), media = 'screen' }
      },
      body{
	 div{ id = "container",
	    div{ id = "header", title = "sitename", "ToyCMS Admin" },
	    div{ id = "mainnav",
	       ul {
		  li{ a{ href = web:link("/admin"), strings.admin_home } },
		  li{ a{ href = web:link("/adduser"), strings.new_user } },
		  li{ a{ href = web:link("/editsection"), strings.new_section } },
		  li{ a{ href = web:link("/editpost"), strings.new_post } },
		  li{ a{ href = web:link("/comments"), strings.manage_comments } },
	       }
	    }, 
            div{ id = "menu",
	       _admin_menu(web, args)
	    },  
	    div{ id = "contents", inner_html },
	    div{ id = "footer", "Copyright 2007 Fabio Mascarenhas" }
	 }
      }
   } 
end

function _admin_menu(web)
   local res = {}
   local user = check_user(web)
   if user then
      res[#res + 1] = ul{ li{ strings.logged_as, 
	    (user.name or user.login) } }
      res[#res + 1] = h3(strings.sections)
      local section_list = {}
      local sections = models.section:find_all()
      for _, section in ipairs(sections) do
	 section_list[#section_list + 1] = 
	    li{ a{ href=web:link("/admin/" .. section.id), section.title } }
      end
      res[#res + 1] = ul(table.concat(section_list,"\n"))
   end
   return table.concat(res, "\n")
end

function render_admin(web, params)
   local section_list
   local sections = models.section:find_all({ order = "id asc" })
   if params.section then
      local section = params.section
      local res_section = {}
      res_section[#res_section + 1] = "<div class=\"blogentry\">\n"
      res_section[#res_section + 1] = h2(strings.section .. ": " ..
					 a{ href = web:link("/editsection/" .. section.id),
					    section.title })
      local posts = models.post:find_all_by_section_id{ section.id,
	 order = "published_at desc" }
      res_section[#res_section + 1] = "<p>"
      for _, post in ipairs(posts) do
	 local in_home, published = "", ""
	 if post.in_home then in_home = " [HOME]" end
	 if post.published then published = " [P]" end
	 res_section[#res_section + 1] = a{ href =
	    web:link("/editpost/" .. post.id), post.title } .. in_home .. 
	    published .. br()
      end
      res_section[#res_section + 1] = "</p>"
      res_section[#res_section + 1] = 
	 p{ a.button{ href = web:link("/editpost?section_id=" .. section.id), 
	    button{ strings.new_post } } }
      res_section[#res_section + 1] = "</div>\n"
      section_list = table.concat(res_section, "\n")      
   elseif next(sections) then
      local res_section = {}
      for _, section in ipairs(sections) do
	 res_section[#res_section + 1] = "<div class=\"blogentry\">\n"
	 res_section[#res_section + 1] = h2(strings.section .. ": " ..
					    a{ href = web:link("/editsection/" .. section.id),
					       section.title })
	 local posts = models.post:find_all_by_section_id{ section.id,
	    order = "published_at desc" }
	 res_section[#res_section + 1] = "<p>"
	 for _, post in ipairs(posts) do
	    local in_home, published = "", ""
	    if post.in_home then in_home = " [HOME]" end
	    if post.published then published = " [P]" end
	    res_section[#res_section + 1] = a{ href =
	       web:link("/editpost/" .. post.id), post.title } .. in_home .. 
	       published .. br()
	 end
	 res_section[#res_section + 1] = "</p>"
	 res_section[#res_section + 1] = 
	    p{ a.button { href = web:link("/editpost?section_id=" .. section.id),
	       button{ strings.new_post } } }
	 res_section[#res_section + 1] = "</div>\n"
      end
      section_list = table.concat(res_section, "\n")
   else
      section_list = strings.no_sections
   end
   return div(section_list)
end

function render_login(web, params)
   local res = {}
   local err_msg = ""
   if params.not_match then
      err_msg = p{ error_message(strings.password_not_match) }
   elseif params.not_found then
      err_msg = p{ error_message(strings.user_not_found) }
   end
   res[#res + 1] = h2"Login"
   res[#res + 1] = err_msg
   res[#res + 1] = form{
      method = "post",
      action = web:link("/login"),
      input{ type = "hidden", name = "link_to", value = params.link_to },
      p{
	 strings.login, br(), input{ type = "text", name = "login",
	    value = params.login or "" },
	 br(), br(),
	 strings.password, br(), input{ type = "password", name = "password" },
	 br(), br(),
	 input{ type = "submit", value = strings.login_button }
      }
   }
   return div(res)
end

function render_add_user(web, params)
   local error_login, error_password, error_name = "", "", ""
   if params.error_login then 
      error_login = error_message(params.error_login) .. br()
   end
   if params.error_password then 
      error_password = error_message(params.error_password) .. br()
   end
   if params.error_name then 
      error_name = error_message(params.error_name) .. br()
   end
   return div{
      h2(strings.new_user),
      form{
	 method = "post",
	 action = web:link("/adduser"),
	 p{
	    strings.login, br(), error_login, input{ type = "text",
	       name = "login", value = params.login }, br(), br(),
	    strings.password, br(), error_password, input{ type = "password",
	       name = "password1" }, br(),
            input{ type = "password", name = "password2" }, br(), br(),
	    strings.name, br(), error_name, input{ type = "text",
	       name = "name", value = params.name }, br(), br(),
	    input{ type = "submit", value = strings.add }
	 }
      },
   }
end

function render_edit_section(web, params)
   local error_title = ""
   if params.error_title then
      error_title = error_message(params.error_title) .. br()
   end
   local page_header, button_text
   if not params.section.id then
      page_header = strings.new_section
      button_text = strings.add
   else
      page_header = strings.edit_section
      button_text = strings.edit
   end
   local action
   local delete
   if params.section.id then
      action = web:link("/editsection/" .. params.section.id)
      delete = form{ method = "post", action = web:link("/deletesection/" ..
							params.section.id), 
	 input{ type = "submit", value = strings.delete } }
   else
      action = web:link("/editsection")
   end
   return div{
      h2(page_header),
      form{
	 method = "post",
	 action = action,
	 p{
	    strings.title, br(), error_title, input{ type = "text",
	       name = "title", value = params.title or params.section.title },
	    br(), br(),
	    strings.description, br(), textarea{ name = "description",
	       rows = "5", cols = "40", params.description or
		  params.section.description }, br(), br(),
	    strings.tag, br(), input{ type = "text", name = "tag",
	       value = params.tag or params.section.tag }, br(), br(),
	    input{ type = "submit", value = button_text }
	 }
      }, delete
   }
end

function render_edit_post(web, params)
   local error_title = ""
   if params.error_title then
      error_title = error_message(params.error_title) .. br()
   end
   local page_header, button_text
   if not params.post.id then
      page_header = strings.new_post
      button_text = strings.add
   else
      page_header = strings.edit_post
      button_text = strings.edit
   end
   local action
   local delete
   if params.post.id then
      action = web:link("/editpost/" .. params.post.id)
      delete = form{ method = "post", action = web:link("/deletepost/" ..
							params.post.id), 
	 input{ type = "submit", value = strings.delete } }
   else
      action = web:link("/editpost")
   end
   local sections = {}
   for _, section in pairs(params.sections) do
      sections[#sections + 1] = option{ value = section.id, 
	 selected = (section.id == (tonumber(params.section_id) or
				    params.post.section_id)) or nil, 
	 section.title }
   end
   sections = "<select name=\"section_id\">" .. table.concat(sections, "\n") ..
      "</select>"
   local comment_status = {}
   for status, text in pairs({ closed = strings.closed, 
			        moderated = strings.moderated,
				unmoderated = strings.unmoderated }) do
      comment_status[#comment_status + 1] = option{ value = status,
	 selected = (status == (params.comment_status or 
				params.post.comment_status)) or nil, text }
   end
   local comment_status = "<select name=\"comment_status\">" ..
      table.concat(comment_status, "\n") .. "</select>"
   return div{
      h2(page_header),
      form{
	 method = "post",
	 action = action,
	 p{
	    strings.section, br(), sections, br(), br(),
	    strings.title, br(), error_title, input{ type = "text",
	       name = "title", value = params.title or params.post.title },
	    br(), br(),
	    strings.index_image, br(), error_title, input{ type = "text",
	       name = "image", value = params.image or params.post.image },
	    br(), br(),
	    strings.external_url, br(), input{ type = "text",
	       name = "external_url", value = params.external_url or 
		  params.post.external_url },
	    br(), br(),
	    strings.abstract, br(), textarea{ name = "abstract",
	       rows = "5", cols = "40", params.abstract or
		  params.post.abstract }, br(), br(),
	    strings.body, br(), textarea{ name = "body",
	       rows = "15", cols = "80", params.body or
		  params.post.body }, br(), br(),
	    strings.comment_status, br(), comment_status, br(), br(),
	    strings.published_at, br(), input{ type = "text",
	       name = "published_at", value = params.published_at or
		  os.date("%d-%m-%Y %H:%M", params.post.published_at) }, br(), br(),
	    input{ type = "checkbox", name = "published", value = "1",
	       checked = params.published or params.post.published or nil },
	    strings.published, br(), br(),
	    input{ type = "checkbox", name = "in_home", value = "1",
	       checked = params.in_home or params.post.in_home or nil },
	    strings.in_home, br(), br(),
	    input{ type = "submit", value = button_text }
	 }
      }, delete
   }
end

function render_manage_comments(web, params)
   local for_mod = {}
   for _, comment in ipairs(params.for_mod) do
      for_mod[#for_mod + 1] = div{
	 p{ id = comment.id, strong{ strings.comment_by, " ", 
	       comment:make_link(), " ",
	       strings.on_post, " ", a{ 
		  href = web:link("/post/" .. comment.post_id), comment.post_title },
	       " ", strings.on, " ", time(comment.created_at), ":" } },
	 markdown(comment.body),
	 p{ form{ action = web:link("/comment/" .. comment.id .. "/approve"),
	       method = "post", input{ type = "submit", value = strings.approve }
	    }, form{ action = web:link("/comment/" .. comment.id .. "/delete"),
	       method = "post", input{ type = "submit", value = strings.delete }
	    }
	 },
      }
   end
   local approved = {}
   for _, comment in ipairs(params.approved) do
      approved[#approved + 1] = div{
	 p{ id = comment.id, strong{ strings.comment_by, " ", 
	       comment:make_link(), " ",
	       strings.on_post, " ", a{ 
		  href = web:link("/post/" .. comment.post_id), comment.post_title },
	       " ", strings.on, " ", time(comment.created_at), ":" } },
	 markdown(comment.body),
	 p{ form{ action = web:link("/comment/" .. comment.id .. "/delete"),
	       method = "post", input{ type = "submit", value = strings.delete }
	 } },
      }
   end
   if #for_mod == 0 then for_mod = { p{ strings.no_comments } } end
   if #approved == 0 then approved = { p{ strings.no_comments } } end
   return div{
      h2(strings.waiting_moderation),
      table.concat(for_mod, "\n"),
      h2(strings.published),
      table.concat(approved, "\n")
   }
end

orbit.htmlify(toycms, "_.+", "admin_layout", "render_.+")