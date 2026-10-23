-- Shared systems run independently in the server and client Lua states.
SCPModeExample = { ticks = 0 }
hook.add("Initialize", "SCPModeExample.Initialize", function()
    SCPModeExample.ticks = 0
end)
hook.add("Think", "SCPModeExample.Think", function()
    SCPModeExample.ticks = SCPModeExample.ticks + 1
end)
