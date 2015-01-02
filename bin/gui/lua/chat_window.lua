
local r = r
local math = math

function addChat(channel, str)
	local chat = r.document:GetElementById("chattext")
	chat.inner_rml = chat.inner_rml .. str .. "<br/>"
	chat.scroll_top = math.huge
end
