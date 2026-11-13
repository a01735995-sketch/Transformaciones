@echo off
setlocal
cd /d "%~dp0"
where gcc >nul 2>nul
if errorlevel 1 (
    echo No se encontro GCC. Agrega C:\msys64\ucrt64\bin a PATH.
    exit /b 1
)
where python >nul 2>nul
if errorlevel 1 (
    echo No se encontro Python 3 en PATH.
    exit /b 1
)
if not exist build mkdir build
gcc -std=c17 -O2 -Wall -Wextra -fopenmp bench_mark_transformaciones.c -o build\transformaciones_bench.exe
if errorlevel 1 exit /b 1
python benchmark_transformaciones.py %*
if errorlevel 1 exit /b 1
echo Listo. Consulta output y results.
endlocal
