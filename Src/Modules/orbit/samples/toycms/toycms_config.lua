module("toycms", package.seeall)

-- Uncomment next line to enable X-Sendfile for sending static files
-- use_xsendfile = true

database = {
  driver = "sqlite3",
  conn_data = { toycms.real_path .. "/blog.db" }
--  driver = "mysql",
--  conn_data = { "blog", "root", "password" }
}

template_name = "blog"

-- Comment this for in-memory caching
cache_path = toycms.real_path .. "/page_cache"

-- Uncomment the following line to set a url prefix
-- prefix = "/foobar"

-- The next two lines are if you want the templates' static files
-- and post images to be served by the web server instead of ToyCMS
-- template_vpath should point to the folder where the template you use is,
-- image_vpath to the folder where ToyCMS stores post's images
-- template_vpath = "/templates"
-- image_vpath = "/images"

strings = {}

strings.pt = {
  closed = "Closed",
  moderated = "Moderated",
  unmoderated = "Unmoderated",
  comment_status = "Comments",
  anonymous_author = "Anonymous",
  external_url = "External URL",
  logged_as = "Logged in as ",
  password_not_match = "Password does not match!",
  user_not_found = "User not found!",
  name = "Name",
  login = "Email",
  password = "Password",
  login_button = "Login",
  new_user = "Add User",
  blank_user = "Login cannot be blank",
  blank_password = "Password cannot be blank",
  blank_name = "Name cannot be blank",
  blank_title = "Title cannot be blank",
  password_mismatch = "Passwords do not match",
  new_section = "Add Section",
  add = "Add",
  edit = "Edit",
  delete = "Delete",
  edit_section = "Edit Section",
  title = "Title",
  description = "Description",
  tag = "Tag",
  sections = "Sections",
  users = "Users",
  posts = "Posts",
  no_sections = "No sections!",
  no_posts = "No posts!",
  admin_console = "Admin console",
  new_post = "Add Post",
  edit_post = "Edit Post",
  published = "Published",
  published_at = "Published at",
  body = "Body",
  abstract = "Abstract",
  section = "Section",
  index_image = "Index image",
  in_home = "Show in home page",
  admin_home = "Admin Home",
  manage_comments = "Manage Comments",
  comment_by = "Comment by",
  on_post = "on post",
  at = "at",
  approve = "Approve",
  waiting_moderation = "Waiting Moderation",
  on = "on",
  no_comments = "There are no comments!"
}

strings.en = {
  closed = "Closed",
  moderated = "Moderated",
  unmoderated = "Unmoderated",
  comment_status = "Comments",
  anonymous_author = "Anonymous",
  external_url = "External URL",
  logged_as = "Logged in as ",
  password_not_match = "Password does not match!",
  user_not_found = "User not found!",
  name = "Name",
  login = "Email",
  password = "Password",
  login_button = "Login",
  new_user = "Add User",
  blank_user = "Login cannot be blank",
  blank_password = "Password cannot be blank",
  blank_name = "Name cannot be blank",
  blank_title = "Title cannot be blank",
  password_mismatch = "Passwords do not match",
  new_section = "Add Section",
  add = "Add",
  edit = "Edit",
  delete = "Delete",
  edit_section = "Edit Section",
  title = "Title",
  description = "Description",
  tag = "Tag",
  sections = "Sections",
  users = "Users",
  posts = "Posts",
  no_sections = "No sections!",
  no_posts = "No posts!",
  admin_console = "Admin console",
  new_post = "Add Post",
  edit_post = "Edit Post",
  published = "Published",
  published_at = "Published at",
  body = "Body",
  abstract = "Abstract",
  section = "Section",
  index_image = "Index image",
  in_home = "Show in home page",
  admin_home = "Admin Home",
  manage_comments = "Manage Comments",
  comment_by = "Comment by",
  on_post = "on post",
  at = "at",
  approve = "Approve",
  waiting_moderation = "Waiting Moderation",
  on = "on",
  no_comments = "There are no comments!"
}

language = "en"

strings = strings[language]

months = {}

months.pt = { "Janeiro", "Fevereiro", "Março", "Abril",
    "Maio", "Junho", "Julho", "Agosto", "Setembro", "Outubro",
    "Novembro", "Dezembro" }

months.en = { "January", "February", "March", "April",
    "May", "June", "July", "August", "September", "October",
    "November", "December" }

month_names = months[language]

weekdays = {}

weekdays.pt = { "Domingo", "Segunda", "Terça", "Quarta",
    "Quinta", "Sexta", "Sábado" }

weekdays.en = { "Sunday", "Monday", "Tuesday", "Wednesday",
    "Thursday", "Friday", "Saturday" }

-- Utility functions

time = {}
date = {}
month = {}

local datetime_mt = { __call = function (tab, date) return tab[language](date) end }

setmetatable(time, datetime_mt)
setmetatable(date, datetime_mt)
setmetatable(month, datetime_mt)

function time.pt(date)
  local time = os.date("%H:%M", date)
  date = os.date("*t", date)
  return date.day .. " de "
    .. months.pt[date.month] .. " de " .. date.year .. " Ã s " .. time
end

function date.pt(date)
  date = os.date("*t", date)
  return weekdays.pt[date.wday] .. ", " .. date.day .. " de "
    .. months.pt[date.month] .. " de " .. date.year
end

function month.pt(month)
  return months.pt[month.month] .. " de " .. month.year
end

local function ordinalize(number)
  if number == 1 then
    return "1st"
  elseif number == 2 then
    return "2nd"
  elseif number == 3 then
    return "3rd"
  else
    return tostring(number) .. "th"
  end
end

function time.en(date)
  local time = os.date("%H:%M", date)
  date = os.date("*t", date)
  return months.en[date.month] .. " " .. ordinalize(date.day) .. " " ..
     date.year .. " at " .. time
end

function date.en(date)
  date = os.date("*t", date)
  return weekdays.en[date.wday] .. ", " .. months.en[date.month] .. " " ..
     ordinalize(date.day) .. " " .. date.year 
end

function month.en(month)
  return months.en[month.month] .. " " .. month.year
end

