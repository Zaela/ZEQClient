------------------------
-- font load manifest --
------------------------

local fonts = {}
local function addFont(path)
	fonts[#fonts + 1] = path
end

--provided fonts
addFont("fonts/LiberationSans-Regular.ttf")
addFont("fonts/LiberationSans-Bold.ttf")
addFont("fonts/LiberationSans-Italic.ttf")
addFont("fonts/LiberationSans-BoldItalic.ttf")

--system-specific fonts
if Windows then
	local folder = "C:/Windows/Fonts/"
	addFont(folder .. "Arial.ttf")
end

return fonts
