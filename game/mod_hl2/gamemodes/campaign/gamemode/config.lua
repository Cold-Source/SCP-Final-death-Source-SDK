-- Menu/server settings are read before the map starts. Return a plain Lua table.
-- Paths are relative to scp; material names (logo/map images) omit materials/ and .vmt.
-- background accepts a local .tga path or a material name; empty keeps the 3D menu map.
return {
    name = "Story", name_ru = "Сюжетный",
    description = "Single-player story", description_ru = "Одиночная сюжетная игра",
    order = 0, coop = false, min_players = 1, max_players = 1, default_players = 1,
    background = "", badge = "",
    logo = "gamepadui/scp_final_death",
    logo_uv_width = 0.52099609375,
    logo_uv_height = 0.935546875,
    all_maps = true, -- false: show only maps listed below (including chapter starts).
    maps = {
        { map = "scp_site19_a1", title = "#SCPSession_Site19", image = "vgui/maps/scp_site19_a1" },
        { map = "hgg", title = "#SCPSession_Hgg", image = "vgui/maps/hgg" },
        { map = "dm_lockdown", title = "#SCPSession_Lockdown", image = "vgui/maps/dm_lockdown" },
    },
    chapters = {
        -- { number = 1, map = "chapter1_start", title = "Chapter 1", title_ru = "Глава 1", image = "vgui/maps/chapter1_start" },
    },
    news = {
        enabled = true,
        repository = "Cold-Source/SCP-FD-News", branch = "main", file = "news.vdf",
        local_file = "gamepadui/news.vdf",
        -- entries = { { title = "News", title_russian = "Новости", body = "Text", body_russian = "Текст", date = "2026-10-23" } },
        -- Set repository = "" to use only local_file or entries.
    },
    -- Custom weapons/entities/interfaces/systems belong to content/lua/, beside gamemode/.
}
