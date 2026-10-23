-- Demonstrates a mode-local interface; hidden until scp_mode_example_hud is used.
local panel
hook.add("CreateDefaultPanels", "SCPModeExample.Panel", function()
    panel = vgui.SCPModeExamplePanel(nil, "SCPModeExample")
end)
concommand.Create("scp_mode_example_hud", function()
    if panel then panel:SetVisible(not panel:IsVisible()) end
end, "Toggle the mode-local example HUD")
