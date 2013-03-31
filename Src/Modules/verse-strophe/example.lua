require "verse-strophe"
--require "verse-strophe.XEP0092" -- Software Version
--require "verse-strophe.XEP0118" -- User Tune

function conn_handler(e)
-- To let people see us as online, we send our initial presence:
	--if e and e.conn then verse.presence():send(e.conn); end

	print "Connected!!"
-- Request someone's version: (requires you to know their resource)
	--verse.XEP0092.request{to = "josh@localhost"}:send(e.conn);
-- Request what song they are listening to: (you must be on their roster)
	--verse.XEP0118.request_last{ to = "josh@localhost" }:send(e.conn);
-- Publish our current song:
	--verse.XEP0118.publish{ artist = "Big Bird and Friends", title = "Sesame Street Theme" }:send(e.conn);
	verse.message({ to = "josh@localhost", type = 'chat' }, "Hello2!"):send(e.conn)
end

-- This sets up our handler for version responses
--verse.hook_event("XEP0092", "response", function (e, p) print(p.name, p.version, p.os); end);

-- and this one sets up our handler for tune notifications
--verse.hook_event("XEP0118", "response", function (e, p) print(tostring(p.from).." is listening to '"..tostring(p.title).."' by '"..tostring(p.artist).."'"); end);

-- If you remove the following line then only errors will be printed
verse.set_log_level("debug");

-- Connect!
conn = verse.connect("service@localhost", "secretpassword", 7002, conn_handler);

-- Here we run verse, which listens to the 
-- connection, and calls our handlers when 
-- something happens
verse.run();
