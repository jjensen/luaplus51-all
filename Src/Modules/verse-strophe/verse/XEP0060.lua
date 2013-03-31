require "verse-strophe"

verse.protocols.XEP0060 = { xmlns = "http://jabber.org/protocol/pubsub" };
local xep = verse.protocols.XEP0060;
xep.events = {
  response = 	{
			{
				name = "iq";
				type = "result";
				xmlns = xep.xmlns;
			};
			
			node = "/iq/*/items/@node";
			payload = "/iq/*/items/item";
		};
}

function xep.request_last(request)
	return verse.iq{ to = request.to, type = "get", id = verse.new_id() }
		:tag("query", { xmlns = xep.xmlns })
			:tag("items", { node = request.node, max_items = request.max_items or 1});
end

function xep.post(post)
	local iq = verse.iq{ to = post.to, type = "set", id = verse.new_id() }
			:tag("pubsub", { xmlns = xep.xmlns })
				:tag("publish", { node = post.node });

	for id, item in ipairs(post.items) do
		iq:tag("item"):child(item):up();
	end
	return iq;
end
xep.publish = xep.post;

function xep.create(req)
	return verse.iq{ to = req.to, type = "set", id = verse.new_id() }
			:tag("pubsub", { xmlns = xep.xmlns })
				:tag("create", { node = req.node }):up()
				:tag("configure");
end

function xep.subscribe(req)
	return verse.iq{ to = req.to, type = "set", id = verse.new_id() }
			:tag("pubsub", { xmlns = xep.xmlns })
				:tag("subscribe", { node = req.node, jid = req.jid });
end

