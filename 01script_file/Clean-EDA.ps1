param(
    [Parameter(Mandatory=$true)][string]$Root,
    [Parameter(ValueFromRemainingArguments=$true)][string[]]$Options = @(),
    [switch]$CopyTemplates
)
# Windows PowerShell 5.1. All paths are literal; no archive extraction or process termination.
$ErrorActionPreference = 'Stop'
$apply = $false
foreach ($option in $Options) {
    switch ($option.ToLowerInvariant()) {
        '/apply' { $apply = $true }
        '/preview' { $apply = $false }
        '/history' { }
        '/build' { }
        '/help' {
            Write-Output 'Usage: this.bat [/preview | /apply] [/help]'
            Write-Output 'Default is preview. The BAT directory is the target, regardless of current working directory.'
            Write-Output 'Learning, production libraries, documents, history, archives and version-control directories are protected.'
            Write-Output 'TXT/CSV/XML/OPJ, CAD designs, libraries, configuration and firmware are preserved.'
            return
        }
        default { throw "Unknown option: $option" }
    }
}
$protected = '^(?i:01AIEDA|01Pad_lib|01Psm_lib|01_3d_step_lib|01captureLib|01brdlib|01aeLib|01env|01script_file|004Document|AUTOlib|PCB_pads_LIB|History|RAR|_sync_records|_maintenance|\.git|\.svn|\.hg|backups?|archives?|originals?|node_modules|\.venv|venv)$'
function Test-ProtectedPath([string]$path) {
    foreach ($part in ($path -split '[\\/]')) { if ($part -match $protected) { return $true } }
    return $false
}
function Assert-RegularPath([string]$path) {
    $full = [IO.Path]::GetFullPath($path)
    if ($full -ne $script:target -and -not $full.StartsWith($script:target + '\', [StringComparison]::OrdinalIgnoreCase)) { throw "Outside target: $full" }
    $cursor = Get-Item -LiteralPath $full -Force
    while ($cursor) {
        if ($cursor.Attributes -band [IO.FileAttributes]::ReparsePoint) { throw "Reparse point refused: $full" }
        $cursor = if ($cursor.PSIsContainer) { $cursor.Parent } else { $cursor.Directory }
    }
    return $full
}
$script:target = [IO.Path]::GetFullPath($Root).TrimEnd('\')
if ($target -eq [IO.Path]::GetPathRoot($target).TrimEnd('\')) { throw 'Drive root refused.' }
foreach ($systemDir in @($env:WINDIR,$env:ProgramFiles,${env:ProgramFiles(x86)},$env:USERPROFILE)) {
    if ($systemDir -and $target -eq $systemDir.TrimEnd('\')) { throw 'System/profile root refused.' }
}
$null = Assert-RegularPath $target
Write-Output "Target: $target; apply=$apply"
if (Test-ProtectedPath $target) { Write-Output 'KEEP protected directory and all descendants.'; return }
if ($apply) {
    $busy = @(Get-Process -ErrorAction SilentlyContinue | Where-Object { $_.ProcessName -match '^(?i:capture|allegro|pcb_editor|padstack_editor|dxp|uv4|iaridepm)$' })
    if ($busy.Count) { throw 'Design/build tools are open. Close them before applying cleanup.' }
}
$stack = New-Object 'System.Collections.Generic.Stack[string]'
$stack.Push($target)
$candidates = @()
while ($stack.Count) {
    $folder = $stack.Pop()
    foreach ($item in @(Get-ChildItem -LiteralPath $folder -Force)) {
        if ($item.Attributes -band [IO.FileAttributes]::ReparsePoint) { continue }
        if ($item.PSIsContainer) {
            if (-not (Test-ProtectedPath $item.FullName)) { $stack.Push($item.FullName) }
            continue
        }
        # Narrow disposable outputs only. Generic TXT/CSV/XML/OPJ/CFG and backup files are never candidates.
        if ($item.Name -notmatch '(?i)\.(tmp|pyc|jrl(?:,\d+)?|log(?:,\d+)?)$') { continue }
        $candidates += [pscustomobject]@{Path=$item.FullName;Length=$item.Length;Time=$item.LastWriteTimeUtc}
    }
}
$removed = 0
foreach ($candidate in $candidates) {
    $relative = $candidate.Path.Substring($target.Length+1)
    if (-not $apply) { Write-Output "WOULD REMOVE $relative"; continue }
    $path = Assert-RegularPath $candidate.Path
    if (Test-ProtectedPath $path) { throw "Protected path: $path" }
    $now = Get-Item -LiteralPath $path -Force
    if ($now.PSIsContainer -or $now.Length -ne $candidate.Length -or $now.LastWriteTimeUtc -ne $candidate.Time) { throw "Changed during preview: $path" }
    Remove-Item -LiteralPath $path -ErrorAction Stop
    $removed++
    Write-Output "REMOVED $relative"
}
if ($CopyTemplates) {
    $libraryRoot = Split-Path -Parent $PSScriptRoot
    foreach ($name in @('BOM.txt','reports.txt')) {
        $source = Join-Path (Join-Path $libraryRoot '01env') $name
        $destination = Join-Path $target $name
        if (Test-Path -LiteralPath $destination) { continue }
        if (-not (Test-Path -LiteralPath $source -PathType Leaf)) { continue }
        $sourceItem = Get-Item -LiteralPath $source -Force
        if ($sourceItem.Attributes -band [IO.FileAttributes]::ReparsePoint) { throw 'Template link refused.' }
        if ($apply) { Copy-Item -LiteralPath $source -Destination $destination -ErrorAction Stop }
        else { Write-Output "WOULD COPY missing template $name" }
    }
}
Write-Output "Candidates=$($candidates.Count); removed=$removed; apply=$apply"
