param(
    [string]$Config = "Debug"
)

$ErrorActionPreference = "Stop"

$buildDir = Join-Path $PSScriptRoot "..\build"

if (-not (Test-Path $buildDir)) {
    New-Item -ItemType Directory -Path $buildDir | Out-Null
}

cmake -B $buildDir -G Ninja -DCMAKE_BUILD_TYPE=$Config
ninja -C $buildDir
