require "verse-strophe"
require "verse-strophe.XEP0060"

local xep = { xmlns = 'http://jabber.org/protocol/tune' };
verse.protocols.XEP0118 = xep;

xep.events = {
  response = 	{
  			{ "XEP0060", "response", node = xep.xmlns }; -- This event is a post-processor of XEP-0060 responses
  									-- (that have the tune namespace)
			artist = "/iq/*/items/item/tune/artist/text()";
			title = "/iq/*/items/item/tune/title/text()";
			source = "/iq/*/items/item/tune/source/text()";
			track = "/iq/*/items/item/tune/track/text()";
			uri = "/iq/*/items/item/tune/uri/text()";
			length = "/iq/*/items/item/tune/length/text()";
			rating = "/iq/*/items/item/tune/rating/text()";

		};
}

function xep.request_last(request)
	return verse.iq{ to = request.to, type = "get", id = verse.new_id() }
		:tag("query", { xmlns = verse.XEP0060.xmlns })
			:tag("items", { node = xep.xmlns, max_items = request.max_items or 1});
end

function xep.publish(tune)
	return verse.iq{to = tune.to, type = "set", id = verse.new_id() }
			:tag("query", { xmlns = verse.XEP0060.xmlns })
				:tag("publish", { node = xep.xmlns })
					:tag("item")
						:tag("tune", { xmlns = xep.xmlns })
							:tag("artist"):text(tune.artist or "Unknown Artist"):up()
							:tag("title"):text(tune.title or "Untitled");
end
