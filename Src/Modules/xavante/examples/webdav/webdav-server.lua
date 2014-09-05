local xavante = require "xavante"

local davHandler = require "xavante.davhandler"
local davFileRepository = require "xavante.davFileRepository"
local davFileProps = require "xavante.davFileProps"

webDir = "."

local simplerules = {
    { 
      match = ".",
      with = davHandler,
      params = {
      	  repos_b = davFileRepository.makeSource(),
      	  props_b = davFileProps.makeProps(),
    	},
    },
}

xavante.HTTP{
	server = { host = "*", port = 8080, },
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

