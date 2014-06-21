local cURL = require("cURL")

c = cURL.easy_init()

c:setopt_url("http://localhost")
postdata = {  
   -- post file from filesystem
   name = {file="post.lua",
	   type="text/plain"},
   -- post file from data variable
   name2 = {file="dummy.html",
	    data="<html><bold>bold</bold></html>",
	    type="text/html"}}
c:post(postdata)
c:perform()

stream_postdata = {
   -- post file from private read function
   name = {file="stream.txt",
	    stream_length="5",
	    type="text/plain"}}

count = 0
c:post(stream_postdata)
c:perform({readfunction=function(n)
			   if (count < 5)  then
				  count = 5
			      return "stream"
			   end
			   return nil
			end})
print("Done")
