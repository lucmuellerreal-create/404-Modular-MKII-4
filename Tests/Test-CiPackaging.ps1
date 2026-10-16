# Tests only the CI package guard, using synthetic PE headers, never plugin binaries.
[CmdletBinding()]
param([Parameter(Mandatory)] [string] $ScratchDirectory)
$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $false
if ($PSVersionTable.PSVersion.Major -lt 7) { throw 'Run this test with PowerShell 7 (pwsh).' }
$scriptSource = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../.github/scripts/Build-Windows.ps1'))
$scratch = [IO.Path]::GetFullPath($ScratchDirectory)
$testRoot = Join-Path $scratch ([guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $testRoot -Force | Out-Null
$pwsh = (Get-Process -Id $PID).Path
$results = @()

foreach ($case in @('missing-bundle', 'missing-binary', 'empty-binary', 'x86-binary', 'arm64-binary', 'corrupt-pe', 'x64-complete')) {
    $fixture = Join-Path $testRoot $case
    $scripts = Join-Path $fixture '.github/scripts'
    New-Item -ItemType Directory -Path $scripts -Force | Out-Null
    Copy-Item -LiteralPath $scriptSource -Destination (Join-Path $scripts 'Build-Windows.ps1')
    $bundle = Join-Path $fixture 'build/SPVinylLab_artefacts/Release/VST3/SP Vinyl Lab.vst3'
    $binary = Join-Path $bundle 'Contents/x86_64-win/SP Vinyl Lab.vst3'
    if ($case -ne 'missing-bundle') {
        New-Item -ItemType Directory -Path (Split-Path $binary) -Force | Out-Null
    }
    if ($case -notin @('missing-bundle', 'missing-binary')) {
        [byte[]] $bytes = New-Object byte[] 512
        $bytes[0] = 0x4d; $bytes[1] = 0x5a
        [BitConverter]::GetBytes([int] 128).CopyTo($bytes, 0x3c)
        [BitConverter]::GetBytes([uint32] 0x00004550).CopyTo($bytes, 128)
        [uint16] $machine = if ($case -eq 'x86-binary') { 0x014c } elseif ($case -eq 'arm64-binary') { 0xaa64 } else { 0x8664 }
        [BitConverter]::GetBytes($machine).CopyTo($bytes, 132)
        [BitConverter]::GetBytes([uint16] 0x2000).CopyTo($bytes, 150)
        [BitConverter]::GetBytes([uint16] 0x020b).CopyTo($bytes, 152)
        if ($case -eq 'corrupt-pe') { $bytes[128] = 0 }
        if ($case -eq 'empty-binary') { $bytes = @() }
        [IO.File]::WriteAllBytes($binary, $bytes)
    }
    if ($case -eq 'x64-complete') {
        $resourceDirectory = Join-Path $bundle 'Contents/Resources'
        New-Item -ItemType Directory -Path $resourceDirectory -Force | Out-Null
        'Synthetic test resource' | Set-Content -LiteralPath (Join-Path $resourceDirectory 'moduleinfo.json')
        $hidden = Join-Path $bundle 'hidden-test.txt'
        'Synthetic hidden test file' | Set-Content -LiteralPath $hidden
        [IO.File]::SetAttributes($hidden, [IO.FileAttributes]::Hidden)
    }
    $manifestDir = Join-Path $fixture 'build/ci/Release'
    New-Item -ItemType Directory -Path $manifestDir -Force | Out-Null
    @{ target = 'SPVinylLab_VST3'; configuration = 'Release'; bundle = $bundle; binary = $binary } |
        ConvertTo-Json | Set-Content -LiteralPath (Join-Path $manifestDir 'vst3-paths.json')
    & $pwsh -NoProfile -File (Join-Path $scripts 'Build-Windows.ps1') -Phase Package *> (Join-Path $fixture 'test-output.log')
    $status = $LASTEXITCODE
    $expectedSuccess = $case -eq 'x64-complete'
    if (($status -eq 0) -ne $expectedSuccess) { throw "Package guard result incorrect: $case, exit $status. See $fixture/test-output.log." }
    if ($expectedSuccess) {
        foreach ($relative in @('Contents/x86_64-win/SP Vinyl Lab.vst3', 'Contents/Resources/moduleinfo.json', 'hidden-test.txt')) {
            if (-not (Test-Path -LiteralPath (Join-Path $fixture "dist/SP Vinyl Lab.vst3/$relative") -PathType Leaf)) {
                throw "Complete bundle staging test failed: $relative"
            }
        }
    } elseif (-not (Test-Path -LiteralPath (Join-Path $fixture 'ci-logs/Package-error.log'))) {
        throw "Failure log not retained: $case"
    }
    $results += [pscustomobject] @{ case = $case; exitCode = $status; passed = $true }
    Write-Host "PASS: $case (exit $status)"
}
$results | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $testRoot 'results.json')
Write-Host "Synthetic fixture results: $testRoot"
