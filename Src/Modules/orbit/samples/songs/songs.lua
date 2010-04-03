#!/usr/bin/env wsapi.cgi

local orbit = require"orbit"
local cosmo = require"cosmo"

local songs = orbit.new()

function songs.index(web)
   local songlist = {
      "Sgt. Pepper's Lonely Hearts Club Band",
      "With a Little Help from My Friends",
      "Lucy in the Sky with Diamonds",
      "Getting Better",
      "Fixing a Hole",
      "She's Leaving Home",
      "Being for the Benefit of Mr. Kite!",
      "Within You Without You",
      "When I'm Sixty-Four",
      "Lovely Rita",
      "Good Morning Good Morning",
      "Sgt. Pepper's Lonely Hearts Club Band (Reprise)",
      "A Day in the Life"
   }
   return songs.layout(songs.render_index({ songs = songlist }))
end

songs:dispatch_get(songs.index, "/")

function songs.layout(inner_html)
  return html{
    head{ title"Song List" },
    body{ inner_html }
  }
end

orbit.htmlify(songs, "layout")

songs.render_index = cosmo.compile[[
	 <h1>Songs</h1>
	    <table>
	    $songs[=[<tr><td>$it</td></tr>]=]
	 </table>  
      ]]

return songs.run
