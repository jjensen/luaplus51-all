require "verse-strophe"

local xep = { xmlns = "http://jabber.org/protocol/pubsub" };
verse.protocols.XEP0163 = xep;

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
