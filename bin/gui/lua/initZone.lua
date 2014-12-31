
local r = r

local doc = r.context:LoadDocument("gui/default.rml")
doc:Show()
doc.id = "zone"

r.document = doc
return doc
