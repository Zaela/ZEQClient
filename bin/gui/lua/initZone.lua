
local r = r

require "chat_window"

local doc = r.context:LoadDocument("gui/default.rml")
doc:Show()
doc.id = "zone"

r.document = doc
return doc
