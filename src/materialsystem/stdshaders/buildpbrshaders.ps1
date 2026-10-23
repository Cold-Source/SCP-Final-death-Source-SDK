param([string]$GameDirectory = (Join-Path $PSScriptRoot '..\..\..\game\mod_hl2'), [uint32]$Threads = 2)
$ErrorActionPreference = 'Stop'
$compiler = Join-Path $PSScriptRoot '..\..\devtools\bin\ShaderCompile2.exe'
foreach ($job in @(@('20b','pbr_vs20b.fxc','pbr_ps20b.fxc'), @('30','pbr_vs30.fxc','pbr_ps30.fxc'))) {
    & $compiler -threads $Threads -ver $job[0] -shaderpath $PSScriptRoot $job[1] $job[2]
    if ($LASTEXITCODE -ne 0) { throw "PBR Shader Model $($job[0]) compilation failed" }
}
$destination = Join-Path $GameDirectory 'shaders\fxc'
New-Item -ItemType Directory -Path $destination -Force | Out-Null
foreach ($shader in @('pbr_vs20','pbr_ps20b','pbr_vs30','pbr_ps30')) {
    Copy-Item -LiteralPath (Join-Path $PSScriptRoot "shaders\fxc\$shader.vcs") -Destination $destination -Force
}
