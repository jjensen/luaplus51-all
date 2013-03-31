require "verse"

verse.protocols.core = {};
local p = verse.protocols.core;

p.events = {
	message = { { name = "message" }, body = "/message/body/text()" };
	presence = { { name = "presence"} };
	iq = { { name = "iq" } };
	};
