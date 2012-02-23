require "xavante"

require "xavante.davhandler"
local davFileRepository = require "xavante.davFileRepository"
local davFileProps = require "xavante.davFileProps"

webDir = arg[1] or "."

local simplerules = {
    {
      match = ".",
      with = xavante.davhandler,
      params = {
      	  repos_b = davFileRepository.makeSource{ rootDir = webDir },
      	  props_b = davFileProps.makeProps(),
    	},
    },
}

xavante.HTTP{
	server = { host = "*", port = arg[2] or 8080, },
	defaultHost = {
		rules = simplerules,
	},
}

-- Displays a message in the console with the used ports
xavante.start_message(function (ports)
	local date = os.date("[%Y-%m-%d %H:%M:%S]")
	print(string.format("%s Xavante started on port(s) %s",
				date, table.concat(ports, ", ")))
end)

xavante.start()

