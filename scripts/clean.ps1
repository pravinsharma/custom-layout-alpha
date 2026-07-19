$ErrorActionPreference = "Continue"

$buildDir = Join-Path $PSScriptRoot "..\build"
if (Test-Path $buildDir) {
    Remove-Item -LiteralPath $buildDir -Recurse -Force
    Write-Host "Removed $buildDir"
} else {
    Write-Host "Nothing to clean"
}
