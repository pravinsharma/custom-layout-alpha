param(
    [string[]]$Args = @()
)

$ErrorActionPreference = "Stop"

& (Join-Path $PSScriptRoot "build.ps1")

$exePath = Join-Path $PSScriptRoot "..\build\VulkanApp.exe"
if (-not (Test-Path $exePath)) {
    Write-Error "Executable not found at $exePath"
}

& $exePath @Args
