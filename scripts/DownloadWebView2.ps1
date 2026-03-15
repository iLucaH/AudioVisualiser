$ErrorActionPreference = "Stop"
$nugetUrl = "https://dist.nuget.org/win-x86-commandline/latest/nuget.exe"
$nugetPath = "$PSScriptRoot/nuget.exe"

if (-not (Test-Path $nugetPath)) {
    Invoke-WebRequest -Uri $nugetUrl -OutFile $nugetPath
}

& $nugetPath install Microsoft.Web.WebView2 -Version 1.0.1901.177 -OutputDirectory "$PSScriptRoot/../packages"