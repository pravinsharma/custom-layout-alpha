param(
    [string[]]$Args = @()
)

$ErrorActionPreference = "Stop"

$exePath = Join-Path $PSScriptRoot "..\build\VulkanApp.exe"
if (-not (Test-Path $exePath)) {
    Write-Error "Executable not found at $exePath. Build the project first with .\scripts\build.ps1"
}

& $exePath @Args
