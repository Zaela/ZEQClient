
local r = r

local doc = r.context:LoadDocument("gui/viewer.rml")
doc:Show()
doc.id = "viewer"

r.document = doc
return doc
