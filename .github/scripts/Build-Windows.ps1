[CmdletBinding()]
param(
    [Parameter(Mandatory)]
    [ValidateSet('Preflight', 'Configure', 'Build', 'Test', 'Package')]
    [string] $Phase
)

$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $false
$projectRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
Set-Location -LiteralPath $projectRoot
$logDirectory = Join-Path $projectRoot 'ci-logs'
New-Item -ItemType Directory -Force -Path $logDirectory | Out-Null
Start-Transcript -Path (Join-Path $logDirectory "$Phase-transcript.log") -Force | Out-Null

function Invoke-LoggedNative {
    param([string] $Executable, [string[]] $Arguments, [string] $LogName)
    & $Executable @Arguments 2>&1 | Tee-Object -FilePath (Join-Path $logDirectory $LogName)
    $nativeExit = $LASTEXITCODE
    if ($nativeExit -ne 0) {
        throw "$Executable failed with exit code $nativeExit. See ci-logs/$LogName."
    }
}

try {
    switch ($Phase) {
        'Preflight' {
            $required = @(
                'CMakeLists.txt', 'CMakePresets.json',
                'Source/PluginProcessor.h', 'Source/PluginProcessor.cpp',
                'Source/PluginEditor.h', 'Source/PluginEditor.cpp',
                'Tests/ProcessorTests.cpp', 'Tests/GuiCapture.cpp'
            )
            foreach ($name in @('VinylNoise', 'WowFlutter', 'VintageCompressor', 'FrequencyRange', 'Saturation')) {
                $required += "Source/DSP/$name.h", "Source/DSP/$name.cpp"
            }
            foreach ($name in @('SkinDesign', 'VinylLookAndFeel', 'LEDDisplay', 'VintageKnob', 'LevelMeter')) {
                $required += "Source/GUI/$name.h", "Source/GUI/$name.cpp"
            }
            foreach ($relative in $required) {
                if (-not (Test-Path -LiteralPath (Join-Path $projectRoot $relative) -PathType Leaf)) {
                    throw "Required repository file missing: $relative. Place the project contents at the repository root."
                }
            }
            Get-ChildItem -LiteralPath Source, Assets -Recurse -File |
                Get-FileHash -Algorithm SHA256 |
                Select-Object Path, Hash |
                ConvertTo-Json | Set-Content -LiteralPath (Join-Path $logDirectory 'source-and-assets.json')
            Invoke-LoggedNative -Executable 'cmake' -Arguments @('--version') -LogName 'cmake-version.log'
            Invoke-LoggedNative -Executable 'git' -Arguments @('--version') -LogName 'git-version.log'
            $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
            if (-not (Test-Path -LiteralPath $vswhere -PathType Leaf)) { throw 'Visual Studio Installer/vswhere.exe missing.' }
            $installation = & $vswhere -latest -products '*' -version '[17.0,18.0)' `
                -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
            if ($LASTEXITCODE -ne 0 -or -not $installation) { throw 'Visual Studio 2022 with MSVC x64 tools missing.' }
            $installation | Set-Content -LiteralPath (Join-Path $logDirectory 'visual-studio.txt')
            "Runner: $env:RUNNER_OS / $env:RUNNER_ARCH; commit: $env:GITHUB_SHA" |
                Set-Content -LiteralPath (Join-Path $logDirectory 'run-info.txt')
            Write-Host 'Source complete. GUI assets are procedural; no external images/fonts/audio files are loaded.'
        }
        'Configure' {
            # The existing preset selects Visual Studio 17 2022 and x64.
            # Empty JUCE_SOURCE_DIR forces the existing FetchContent path on CI.
            Invoke-LoggedNative -Executable 'cmake' -Arguments @(
                '--preset', 'vs2022-x64', '-T', 'v143',
                '-DJUCE_SOURCE_DIR=', '-DSPV_BUILD_TESTS=ON'
            ) -LogName 'configure.log'
            # Verify the immutable commit behind the release tag used in CMake.
            $juceSource = Join-Path $projectRoot 'build/_deps/juce-src'
            $commit = & git -C $juceSource rev-parse HEAD
            if ($LASTEXITCODE -ne 0) { throw 'Cannot read fetched JUCE commit.' }
            $expected = '51a8a6d7aeae7326956d747737ccf1575e61e209'
            if ($commit.Trim() -ne $expected) { throw "JUCE 8.0.6 commit mismatch: $commit (expected $expected)." }
            $commit | Set-Content -LiteralPath (Join-Path $logDirectory 'juce-commit.txt')
            if (-not (Test-Path -LiteralPath 'build/ci/Release/vst3-paths.json' -PathType Leaf)) {
                throw 'CMake did not generate the VST3 target/path manifest.'
            }
        }
        'Build' {
            $binaryLog = Join-Path $logDirectory 'msbuild.binlog'
            Invoke-LoggedNative -Executable 'cmake' -Arguments @(
                '--build', 'build', '--config', 'Release', '--parallel', '2',
                '--target', 'SPVinylLab_VST3', 'SPVinylLabTests', 'SPVinylLabGuiCapture',
                '--', "/bl:$binaryLog"
            ) -LogName 'build.log'
        }
        'Test' {
            Invoke-LoggedNative -Executable 'ctest' -Arguments @(
                '--preset', 'release', '--no-tests=error', '--timeout', '120'
            ) -LogName 'tests.log'
        }
        'Package' {
            $manifest = Get-Content -LiteralPath 'build/ci/Release/vst3-paths.json' -Raw | ConvertFrom-Json
            if ($manifest.target -ne 'SPVinylLab_VST3' -or $manifest.configuration -ne 'Release') {
                throw 'Unexpected CMake target or configuration in manifest.'
            }
            $bundle = [IO.Path]::GetFullPath($manifest.bundle)
            $binary = [IO.Path]::GetFullPath($manifest.binary)
            $buildPrefix = [IO.Path]::GetFullPath((Join-Path $projectRoot 'build')) + [IO.Path]::DirectorySeparatorChar
            if (-not $bundle.StartsWith($buildPrefix, [StringComparison]::OrdinalIgnoreCase)) {
                throw 'Bundle path is outside the build directory.'
            }
            if (-not (Test-Path -LiteralPath $bundle -PathType Container)) { throw "VST3 bundle missing: $bundle" }
            if ([IO.Path]::GetFileName($bundle) -ne 'SP Vinyl Lab.vst3') { throw "Unexpected bundle name: $bundle" }
            $expectedBinary = Join-Path $bundle 'Contents/x86_64-win/SP Vinyl Lab.vst3'
            if ($binary -ne [IO.Path]::GetFullPath($expectedBinary)) { throw "Unexpected VST3 binary path: $binary" }
            if (-not (Test-Path -LiteralPath $binary -PathType Leaf)) { throw "VST3 binary missing: $binary" }
            $bytes = [IO.File]::ReadAllBytes($binary)
            if ($bytes.Length -lt 256 -or $bytes[0] -ne 0x4d -or $bytes[1] -ne 0x5a) { throw 'VST3 binary is empty or has no DOS/PE header.' }
            $pe = [BitConverter]::ToInt32($bytes, 0x3c)
            if ($pe -lt 64 -or $pe -gt ($bytes.Length - 26)) { throw 'Invalid PE header offset.' }
            if ([BitConverter]::ToUInt32($bytes, $pe) -ne 0x00004550 `
                -or [BitConverter]::ToUInt16($bytes, $pe + 4) -ne 0x8664 `
                -or [BitConverter]::ToUInt16($bytes, $pe + 24) -ne 0x020b `
                -or ([BitConverter]::ToUInt16($bytes, $pe + 22) -band 0x2000) -eq 0) {
                throw 'VST3 binary is not an AMD64 PE32+ DLL.'
            }
            # Copy the entire bundle, including Contents/Resources and hidden files.
            $distribution = Join-Path $projectRoot 'dist'
            New-Item -ItemType Directory -Force -Path $distribution | Out-Null
            $staged = Join-Path $distribution 'SP Vinyl Lab.vst3'
            if (Test-Path -LiteralPath $staged) { throw 'Staged bundle already exists; use a clean CI workspace.' }
            Copy-Item -LiteralPath $bundle -Destination $staged -Recurse -Force
            $sourceFiles = @(Get-ChildItem -LiteralPath $bundle -Recurse -Force -File)
            $stagedFiles = @(Get-ChildItem -LiteralPath $staged -Recurse -Force -File)
            if ($sourceFiles.Count -ne $stagedFiles.Count) { throw 'Incomplete bundle copy.' }
            foreach ($file in $sourceFiles) {
                $relative = [IO.Path]::GetRelativePath($bundle, $file.FullName)
                $destination = Join-Path $staged $relative
                if ((Get-FileHash -LiteralPath $file.FullName).Hash -ne (Get-FileHash -LiteralPath $destination).Hash) {
                    throw "Bundle copy hash mismatch: $relative"
                }
            }
            Get-ChildItem -LiteralPath $staged -Recurse -Force -File |
                Get-FileHash -Algorithm SHA256 |
                Select-Object Path, Hash |
                ConvertTo-Json | Set-Content -LiteralPath (Join-Path $logDirectory 'bundle-sha256.json')
            Write-Host "Verified AMD64 VST3: $binary"
            Write-Host "Complete bundle staged: $staged"
        }
    }
} catch {
    $_ | Out-String | Add-Content -LiteralPath (Join-Path $logDirectory "$Phase-error.log")
    throw
} finally {
    Stop-Transcript | Out-Null
}
