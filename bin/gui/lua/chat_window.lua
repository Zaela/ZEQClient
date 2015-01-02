
local r = r
local gGUI = gGUI

local chat = r.document:GetElementById("chattext")
gGUI.registerChatWindow(chat, gGUI.chat_channels.CHANNEL_ALL)
