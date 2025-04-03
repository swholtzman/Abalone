# build_with_nuitka.ps1
$ErrorActionPreference = "Stop"

$nuitkaArgs = @(
    "nuitka",
    "--standalone",
    "--enable-plugin=pyqt5",
    "--include-package=python_gui",
    "--include-data-dir=public=public",
    "python_gui\main_app.py"
)

Write-Host "Running: $($nuitkaArgs -join ' ')"
$cmd = $nuitkaArgs[0]
$args = $nuitkaArgs[1..($nuitkaArgs.Length - 1)]
& $cmd $args
