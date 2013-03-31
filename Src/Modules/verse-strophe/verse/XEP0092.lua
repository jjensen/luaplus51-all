require "verse-strophe"

verse.protocols.XEP0092 = {};
local xep = verse.protocols.XEP0092;
xep.events = {
  request = 	{
			{
				name = "iq";
				type = "get";
				xmlns = "jabber:iq:version";
			};
		};
		
  response = 	{
			{
				name = "iq";
				type = "result";
				xmlns = "jabber:iq:version";
			};
			
			name = "/iq/query/name/text()";
			version = "/iq/query/version/text()";
			os = "/iq/query/os/text()";
		}
}

function xep.request(request)
	return verse.iq{ to = request.to, type = "get", id = verse.new_id() }
		:tag("query", { xmlns = "jabber:iq:version" });
end

function xep.response(response)
	return verse.iq{ to = response.to, type = "get", id = response.id }
		:tag("query", { xmlns = "jabber:iq:version" })
			:tag("name"):text(response.name):up()
			:tag("version"):text(response.version):up()
			:tag("os"):text(response.os);
end
