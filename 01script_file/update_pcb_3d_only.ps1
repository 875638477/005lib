param(
    [Parameter(Mandatory=$true)][string]$Board,
    [string]$Output='',
    [string]$CadenceRoot='C:/Program0/Cadence/SPB172',
    [switch]$Lga1700
)
$ErrorActionPreference='Stop'
$libraryRoot=(Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path.Replace('\','/')
$inputBoard=(Resolve-Path -LiteralPath $Board).Path
if(-not $Output){$Output=Join-Path (Split-Path $inputBoard -Parent) (([IO.Path]::GetFileNameWithoutExtension($inputBoard))+'.3d.brd')}
$outputBoard=[IO.Path]::GetFullPath($Output)
if(Test-Path -LiteralPath $outputBoard){throw 'Output already exists; select a new output file.'}
if($inputBoard -eq $outputBoard){throw 'Input and output must differ.'}
$utility=Join-Path $CadenceRoot 'tools/bin/refresh_symbol.exe'
if(-not(Test-Path -LiteralPath $utility)){throw 'refresh_symbol.exe not found.'}
$work=Join-Path (Split-Path $outputBoard -Parent) ('3d_update_'+(Get-Date -Format 'yyyyMMdd_HHmmss'))
$pcbEnv=Join-Path $work 'pcbenv';New-Item -ItemType Directory -Path $pcbEnv -Force | Out-Null
$psm=@("`"$libraryRoot/01Psm_lib`"")
if($Lga1700){$psm=@("`"$libraryRoot/01Psm_lib/LGA1700_MOD26_20261106`"","`"$libraryRoot/01Psm_lib`"")}
$config='source $TELENV'+"`nset psmpath = ( "+($psm -join ' ')+" )`nset padpath = ( `"$libraryRoot/01Pad_lib`" )`nset steppath = ( `"$libraryRoot/01_3d_step_lib`" )`n"
[IO.File]::WriteAllText((Join-Path $pcbEnv 'env'),$config,(New-Object Text.UTF8Encoding($false)))
$before=(Get-FileHash -LiteralPath $inputBoard -Algorithm SHA256).Hash
$env:ALLEGRO_PCBENV=$pcbEnv
$env:CDSROOT=$CadenceRoot
$previousLocation=Get-Location
try{
    Set-Location -LiteralPath $work
    & $utility -g -p -m $inputBoard $outputBoard *> (Join-Path $work 'refresh_symbol.log')
    if($LASTEXITCODE -ne 0 -or -not(Test-Path -LiteralPath $outputBoard)){throw 'STEP-only refresh failed; inspect the log.'}
}finally{Set-Location -LiteralPath $previousLocation.Path}
if((Get-FileHash -LiteralPath $inputBoard -Algorithm SHA256).Hash -ne $before){throw 'Input board changed unexpectedly.'}
Write-Output "STEP-only candidate: $outputBoard"
Write-Output "Reopen and verify before replacing the working board. Log: $work"
