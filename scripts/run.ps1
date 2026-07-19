param(
    [string]$Example = "",
    [string[]]$Args = @()
)

$ErrorActionPreference = "Stop"

if ($Example) {
    $exePath = Join-Path $PSScriptRoot "..\build\examples\flex-example-$Example.exe"
    if (-not (Test-Path $exePath)) {
        Write-Error "Example executable not found at $exePath. Build examples first with .\scripts\build.ps1"
    }
} else {
    $exePath = Join-Path $PSScriptRoot "..\build\VulkanApp.exe"
    if (-not (Test-Path $exePath)) {
        Write-Error "Executable not found at $exePath. Build the project first with .\scripts\build.ps1"
    }
}

& $exePath @Args
