param(
    [Parameter(Mandatory=$true)][string]$EngineRoot,
    [ValidateSet("Build", "Prepare", "Test", "Package", "Editor", "All")][string]$Action = "All"
)
$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest
$ProjectRoot = Split-Path $PSScriptRoot -Parent
$Project = Join-Path $ProjectRoot "NOSTALGIA.uproject"
$Pin = Get-Content (Join-Path $ProjectRoot "Config/EngineVersion.json") -Raw | ConvertFrom-Json
$BuildVersionPath = Join-Path $EngineRoot "Engine/Build/Build.version"
if (!(Test-Path $BuildVersionPath)) { throw "UE installation missing: $BuildVersionPath" }
$Version = Get-Content $BuildVersionPath -Raw | ConvertFrom-Json
if ($Version.MajorVersion -ne $Pin.major -or $Version.MinorVersion -ne $Pin.minor -or $Version.PatchVersion -ne $Pin.patch) {
    throw "Requires UE $($Pin.major).$($Pin.minor).$($Pin.patch); installation version differs. No automatic conversion."
}
$Editor = Join-Path $EngineRoot "Engine/Binaries/Win64/UnrealEditor.exe"
$EditorCmd = Join-Path $EngineRoot "Engine/Binaries/Win64/UnrealEditor-Cmd.exe"
$Build = Join-Path $EngineRoot "Engine/Build/BatchFiles/Build.bat"
$UAT = Join-Path $EngineRoot "Engine/Build/BatchFiles/RunUAT.bat"
$Output = Join-Path $ProjectRoot "Artifacts/Unreal"
New-Item -ItemType Directory -Force -Path $Output | Out-Null
Copy-Item $BuildVersionPath (Join-Path $Output "Build.version")

function Invoke-Checked([string]$Program, [string[]]$Arguments) {
    & $Program @Arguments
    if ($LASTEXITCODE -ne 0) { throw "$Program failed with exit code $LASTEXITCODE" }
}

if ($Action -in @("Build", "All")) {
    Invoke-Checked -Program $Build -Arguments @("NOSTALGIAEditor", "Win64", "Development", "-Project=$Project", "-WaitMutex", "-NoHotReloadFromIDE")
}
if ($Action -in @("Prepare", "All")) {
    $PrepareReport = Join-Path $Output "prepare.json"
    if (Test-Path $PrepareReport) { Remove-Item $PrepareReport }
    Invoke-Checked -Program $EditorCmd -Arguments @($Project, "/Engine/Maps/Entry", "-run=pythonscript",
        "-script=$PSScriptRoot/prepare_unreal.py", "-unattended", "-nullrhi", "-nop4", "-stdout", "-abslog=$Output/prepare.log")
    if (!(Test-Path $PrepareReport)) { throw "Editor exited without a fresh successful preparation report. Read Artifacts/Unreal/prepare.log." }
    $Prepared = Get-Content $PrepareReport -Raw | ConvertFrom-Json
    if (!$Prepared.prepared) { throw "Preparation incomplete" }
}
if ($Action -in @("Test", "Package", "Editor", "All")) {
    if (!(Test-Path (Join-Path $ProjectRoot "Content/Maps/EndlessSummer.umap"))) {
        throw "Map missing. Run -Action Prepare after building the editor target."
    }
}
if ($Action -in @("Test", "All")) {
    $ReportDir = Join-Path $Output ("Automation_" + [DateTime]::UtcNow.ToString("yyyyMMdd_HHmmss_fff"))
    Invoke-Checked -Program $EditorCmd -Arguments @($Project, "/Game/Maps/EndlessSummer", "-unattended", "-nullrhi", "-nop4", "-nosplash",
        "-ExecCmds=Automation RunTests Nostalgia.", "-TestExit=Automation Test Queue Empty", "-ReportExportPath=$ReportDir", "-abslog=$ReportDir.log")
    $Index = Join-Path $ReportDir "index.json"
    if (!(Test-Path $Index)) { throw "No automation report from this invocation; zero tests is not success." }
    $Results = Get-Content $Index -Raw | ConvertFrom-Json
    $Expected = @("Nostalgia.Core.CausalEscapeChain", "Nostalgia.Data.EventCatalogValidation", "Nostalgia.Save.UnrealSerialization", "Nostalgia.Assets.PreparedPackages")
    foreach ($Name in $Expected) {
        $Match = @($Results.tests | Where-Object { $_.fullTestPath -eq $Name })
        if ($Match.Count -ne 1 -or $Match[0].state -ne "Success") { throw "Required test did not pass: $Name" }
    }
    if ($Results.failed -ne 0) { throw "Automation report contains failures" }
    Write-Host "Verified $($Expected.Count) required Unreal automation tests in $Index. Human PIE acceptance remains separate."
}
if ($Action -eq "Package") {
    Invoke-Checked -Program $UAT -Arguments @("BuildCookRun", "-project=$Project", "-noP4", "-platform=Win64",
        "-clientconfig=Development", "-build", "-cook", "-map=/Game/Maps/EndlessSummer", "-stage", "-pak", "-archive",
        "-archivedirectory=$ProjectRoot/Builds/Win64", "-utf8output")
}
if ($Action -eq "Editor") { Invoke-Checked -Program $Editor -Arguments @($Project, "/Game/Maps/EndlessSummer") }
