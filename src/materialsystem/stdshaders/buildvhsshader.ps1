param([string]$GameDirectory = (Join-Path $PSScriptRoot '..\..\..\game\mod_hl2'), [uint32]$Threads = 2)
$ErrorActionPreference = 'Stop'
$compiler = Join-Path $PSScriptRoot '..\..\devtools\bin\ShaderCompile2.exe'
& $compiler -threads $Threads -ver 20b -shaderpath $PSScriptRoot scp_vhs_ps20b.fxc
if ($LASTEXITCODE -ne 0) { throw 'VHS shader compilation failed' }
$shader = Join-Path $PSScriptRoot 'shaders\fxc\scp_vhs_ps20b.vcs'
if (!(Test-Path -LiteralPath $shader)) { throw 'VHS shader output is missing' }
$destination = Join-Path $GameDirectory 'shaders\fxc'
New-Item -ItemType Directory -Path $destination -Force | Out-Null
Copy-Item -LiteralPath $shader -Destination $destination -Force
