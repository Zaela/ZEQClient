
local r = r

local doc = r.context:LoadDocument("gui/default.rml")
doc:Show()
doc.id = "zone"

r.document = doc

require "chat_window"

return doc
