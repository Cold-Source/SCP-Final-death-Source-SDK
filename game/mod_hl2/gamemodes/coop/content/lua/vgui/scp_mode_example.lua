-- A mode-local VGUI class. Existing Lua vgui API: register(table, name, base).
local Panel = {}
function Panel:Init()
    self:SetBounds(24, 24, 240, 36)
    self:SetMouseInputEnabled(false)
    self:SetKeyBoardInputEnabled(false)
    self:SetPaintBackgroundEnabled(false)
    self:SetVisible(false)
end
function Panel:Paint()
    local width, height = self:GetWide(), self:GetTall()
    surface.DrawSetTexture(-1)
    surface.DrawSetColor(0, 0, 0, 180)
    surface.DrawFilledRect(0, 0, width, height)
    surface.DrawSetColor(255, 255, 255, 230)
    surface.DrawOutlinedRect(0, 0, width, height)
    local fraction = (SCPModeExample.ticks % 240) / 240
    surface.DrawFilledRect(6, 6, 6 + math.floor((width - 12) * fraction), height - 6)
end
vgui.register(Panel, "SCPModeExamplePanel", "Panel")
