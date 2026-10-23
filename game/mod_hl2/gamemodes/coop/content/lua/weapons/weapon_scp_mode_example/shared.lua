-- A new weapon class, available only while this gamemode's content is mounted.
-- Uses the existing scripted revolver base; replace models/attacks here for your gun.
SWEP.printname = "SCP mode example"
SWEP.viewmodel = "models/weapons/v_357.mdl"
SWEP.playermodel = "models/weapons/w_357.mdl"
SWEP.clip_size = 6
SWEP.default_clip = 6
SWEP.primary_ammo = "357"
SWEP.damage = 75

function SWEP:Reload()
    -- The stock example base casts to HL2MPPlayer; our game also has HL2 players.
    return self:DefaultReload(self:GetMaxClip1(), self:GetMaxClip2(), 182)
end
