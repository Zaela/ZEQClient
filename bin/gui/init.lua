
package.path = package.path .. ";./gui/?.lua;./gui/lua/?.lua"

r = {}

local dim = Vector2i.new(screenWidth, screenHeight)
r.context = rocket:CreateContext("context", dim)

return r.context
