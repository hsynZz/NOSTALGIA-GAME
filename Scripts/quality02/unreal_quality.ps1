param(
    [Parameter(Mandatory=$true)][string]$EngineRoot,
    [ValidateSet("Build", "Import", "Editor", "Play")][string]$Action = "Editor"
)
$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest
$ProjectRoot = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$Project = Join-Path $ProjectRoot "NOSTALGIA.uproject"
$BuildVersion = Get-Content (Join-Path $EngineRoot "Engine/Build/Build.version") -Raw | ConvertFrom-Json
if ($BuildVersion.MajorVersion -ne 5 -or $BuildVersion.MinorVersion -ne 8 -or $BuildVersion.PatchVersion -ne 0) {
    throw "Quality 0.2 requires the pinned UE 5.8.0. No automatic project conversion."
}
if ($Action -eq "Build") {
    & (Join-Path $ProjectRoot "Scripts/unreal.ps1") -EngineRoot $EngineRoot -Action Build
    exit
}
$Output = Join-Path $ProjectRoot "Artifacts/Quality02"
New-Item -ItemType Directory -Force -Path $Output | Out-Null
$Editor = Join-Path $EngineRoot "Engine/Binaries/Win64/UnrealEditor.exe"
if ($Action -eq "Import") {
    $Report = Join-Path $Output "unreal-import.json"
    if (Test-Path $Report) { Remove-Item $Report }
    & (Join-Path $EngineRoot "Engine/Binaries/Win64/UnrealEditor-Cmd.exe") $Project "/Engine/Maps/Entry" `
        "-CustomConfig=Quality02" "-run=pythonscript" "-script=$ProjectRoot/Scripts/prepare_quality02.py" `
        "-unattended" "-nop4" "-stdout" "-abslog=$Output/import.log"
    if ($LASTEXITCODE -ne 0) { throw "Unreal import failed; read $Output/import.log" }
    if (!(Test-Path $Report)) { throw "Unreal exited without an import report" }
    $Result = Get-Content $Report -Raw | ConvertFrom-Json
    if (!$Result.prepared) { throw "Map preparation incomplete. See $Report" }
    Write-Output "Prepared map: $($Result.map). Visual and gameplay acceptance still pending."
    exit
}
$Report = Join-Path $Output "unreal-import.json"
if (!(Test-Path $Report)) { throw "Run -Action Import first on the UE workstation." }
$Result = Get-Content $Report -Raw | ConvertFrom-Json
if (!$Result.prepared) { throw "Import was not successful." }
$Args = @($Project, $Result.map, "-CustomConfig=Quality02", "-d3d12", "-sm6", "-log")
if ($Action -eq "Play") { $Args += "-game" }
& $Editor @Args
if ($LASTEXITCODE -ne 0) { throw "Unreal exited with code $LASTEXITCODE" }
