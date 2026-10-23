param(
    [Parameter(Mandatory=$true)][string]$EngineRoot
)
# Read-only checks. Does not install software, edit .uproject, replace MCP config, or open a firewall.
$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest
$ProjectRoot = Split-Path $PSScriptRoot -Parent
$ProjectPath = Join-Path $ProjectRoot "NOSTALGIA.uproject"
$Pin = Get-Content (Join-Path $ProjectRoot "Config/EngineVersion.json") -Raw | ConvertFrom-Json
$Project = Get-Content $ProjectPath -Raw | ConvertFrom-Json
$Checks = [System.Collections.Generic.List[object]]::new()

function Record([string]$Name, [bool]$Passed, [bool]$Required, [string]$Detail) {
    $Checks.Add([pscustomobject]@{name=$Name; passed=$Passed; required=$Required; detail=$Detail})
    $State = if ($Passed) { "OK" } elseif ($Required) { "BLOCKED" } else { "CHECK" }
    Write-Host "[$State] ${Name}: $Detail"
}

Write-Host "Project: $ProjectPath"
Write-Host "Engine:  $EngineRoot"
$OnWindows = [Environment]::OSVersion.Platform -eq [PlatformID]::Win32NT
Record "Windows" $OnWindows $true ([Environment]::OSVersion.VersionString)
Record "Project association" ($Project.EngineAssociation -eq $Pin.association) $true $Project.EngineAssociation
$VersionFile = Join-Path $EngineRoot "Engine/Build/Build.version"
if (Test-Path $VersionFile) {
    $Version = Get-Content $VersionFile -Raw | ConvertFrom-Json
    $Actual = "$($Version.MajorVersion).$($Version.MinorVersion).$($Version.PatchVersion)"
    $Expected = "$($Pin.major).$($Pin.minor).$($Pin.patch)"
    Record "Engine version" ($Actual -eq $Expected) $true "Installed=$Actual; expected=$Expected"
} else { Record "Engine version" $false $true "Missing $VersionFile" }
foreach ($File in @("Engine/Binaries/Win64/UnrealEditor.exe", "Engine/Binaries/Win64/UnrealEditor-Cmd.exe", "Engine/Build/BatchFiles/Build.bat")) {
    Record $File (Test-Path (Join-Path $EngineRoot $File)) $true "Required for editor build/preparation"
}

$VSWhere = if (${env:ProgramFiles(x86)}) { Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio/Installer/vswhere.exe" } else { "" }
if ($VSWhere -and (Test-Path $VSWhere)) {
    $VS = & $VSWhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    Record "MSVC C++ tools" ($LASTEXITCODE -eq 0 -and ![string]::IsNullOrWhiteSpace(($VS -join ''))) $true ($VS -join '; ')
} else { Record "MSVC C++ tools" $false $true "Visual Studio Installer/vswhere absent; install the C++ game development workload" }
$SdkRoot = if (${env:ProgramFiles(x86)}) { Join-Path ${env:ProgramFiles(x86)} "Windows Kits/10/Include" } else { "" }
$SDKs = @(if ($SdkRoot -and (Test-Path $SdkRoot)) { Get-ChildItem $SdkRoot -Directory | Where-Object { Test-Path (Join-Path $_.FullName "um/Windows.h") } })
Record "Windows SDK headers" ($SDKs.Count -gt 0) $true (($SDKs | ForEach-Object { $_.Name }) -join ', ')

foreach ($PluginName in @("ModelContextProtocol", "AllToolsets")) {
    $Found = @()
    foreach ($PluginRoot in @((Join-Path $EngineRoot "Engine/Plugins"), (Join-Path $ProjectRoot "Plugins"))) {
        if (Test-Path $PluginRoot) { $Found += @(Get-ChildItem $PluginRoot -Filter "$PluginName.uplugin" -File -Recurse -ErrorAction Stop) }
    }
    $Enabled = @($Project.Plugins | Where-Object { $_.Name -eq $PluginName -and $_.Enabled }).Count -gt 0
    Record "$PluginName installed" ($Found.Count -gt 0) $false "Needed for live MCP, not for ordinary manual playtesting"
    Record "$PluginName project entry" $Enabled $false "Check this project's Plugins panel; installation alone does not enable tools"
}

$Processes = @(Get-Process -Name UnrealEditor -ErrorAction SilentlyContinue)
Record "Editor process" ($Processes.Count -gt 0) $false "Running editors=$($Processes.Count); a process is not proof of MCP reachability"
if ($OnWindows) {
    try {
        $Machine = Get-CimInstance Win32_ComputerSystem
        Write-Host ("Physical memory: {0:N1} GiB; start builds with at most 2 parallel actions" -f ($Machine.TotalPhysicalMemory / 1GB))
    } catch { Write-Host "Optional memory query unavailable. Start builds with at most 2 parallel actions." }
}

$Output = Join-Path $ProjectRoot "Artifacts/WindowsPreflight"
New-Item -ItemType Directory -Force -Path $Output | Out-Null
$Report = [pscustomobject]@{
    utc=[DateTime]::UtcNow.ToString("o"); project=$ProjectPath; engine=$EngineRoot; checks=$Checks.ToArray();
    unreal_build="not_run"; live_mcp="not_verified"; gameplay="not_run"
}
$Report | ConvertTo-Json -Depth 6 | Set-Content (Join-Path $Output "report.json") -Encoding UTF8
Write-Host "Local report: $Output/report.json"
if (@($Checks | Where-Object { $_.required -and !$_.passed }).Count -gt 0) { exit 1 }
Write-Host "Prerequisite checks passed. UHT/UBT still need to establish actual toolchain compatibility."
