return {
    name = "Deathmatch", name_ru = "Бой насмерть", coop = true,
    min_players = 2, max_players = 4, default_players = 4,
    background = "materials/gamepadui/coop_background.tga", badge = "Deathmatch", badge_ru = "Бой насмерть",
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
