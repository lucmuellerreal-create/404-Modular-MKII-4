$ErrorActionPreference = 'Stop'
Set-Location -LiteralPath $PSScriptRoot
$logDir = Join-Path $PSScriptRoot 'BuildLogs'
New-Item -ItemType Directory -Force -Path $logDir | Out-Null
Start-Transcript -Path (Join-Path $logDir 'Release.log') -Force
try {
    $cmakeCommand = Get-Command cmake -ErrorAction SilentlyContinue
    if (-not $cmakeCommand) {
        throw 'CMake fehlt im PATH. Visual Studio 2022 mit Desktopentwicklung mit C++, MSVC v143, Windows SDK und CMake installieren oder dessen bin-Verzeichnis zum PATH hinzufuegen.'
    }
    & $cmakeCommand.Source --preset vs2022-x64
    if ($LASTEXITCODE -ne 0) { throw "CMake-Konfiguration fehlgeschlagen: $LASTEXITCODE" }
    & $cmakeCommand.Source --build --preset release --parallel
    if ($LASTEXITCODE -ne 0) { throw "Release-Build fehlgeschlagen: $LASTEXITCODE" }
    $ctest = Join-Path (Split-Path $cmakeCommand.Source) 'ctest.exe'
    if (-not (Test-Path -LiteralPath $ctest)) { throw 'ctest.exe neben cmake.exe fehlt.' }
    & $ctest --preset release
    if ($LASTEXITCODE -ne 0) { throw "Tests fehlgeschlagen: $LASTEXITCODE" }
    Write-Host 'Release-Build und Tests erfolgreich.'
} catch {
    Write-Host $_ -ForegroundColor Red
    exit 1
} finally {
    Stop-Transcript
}
