--========= Copyleft © 2010, Team Sandbox, Some rights reserved. ============--
--
-- Purpose:
--
-- $NoKeywords: $
--===========================================================================--

local vgui = vgui;
local ScreenWidth = UTIL.ScreenWidth
local ScreenHeight = UTIL.ScreenHeight

local CBuildSubMenu = {
	m_hFonts = {}
}

function CBuildSubMenu:Init(pViewPort)
	self:SetProportional(true);
	-- Make it screen sized


	self:SetAutoDelete( false );
	
	self.m_pViewPort = pViewPort;

end

function CBuildSubMenu:ApplySchemeSettings( pScheme )
	self:SetBgColor( Color(1, 255, 1, 80) );
end


function CBuildSubMenu:Update()
end
function CBuildSubMenu:OnClose()
	self.BaseClass.OnClose( self );
end

vgui.register( CBuildSubMenu, "CBuildSubMenu", "Frame" )

