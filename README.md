# Transformaciones BMP con OpenMP

Procesamiento de imágenes BMP en C y medición de rendimiento con Python. El programa reparte un lote de imágenes entre hilos de OpenMP y genera cuatro resultados por imagen: espejo horizontal en color y en gris, y giro de 180° en color y en gris.

El código se extrajo de `Transformaciones_OpenMP` del archivo `Benchmarks_BMP_Windows.zip`. La versión recibida se conserva en [`original/`](original/); esta versión añade rutas configurables, validaciones, compatibilidad con alturas BMP negativas y resultados de benchmark con contexto reproducible. La otra biblioteca incluida en el ZIP se utiliza como referencia en la documentación y no forma parte de este repositorio.

**Documentación completa:** [Wiki](https://github.com/a01735995-sketch/Transformaciones/wiki) · [Copia de las páginas en el repositorio](docs/wiki/Home.md).

## Requisitos

- Compilador de C17 con OpenMP: GCC en Windows/Linux o Apple Clang con `libomp` en macOS.
- Python **3.11 o posterior**, sin paquetes adicionales.
- Git para clonar el proyecto.
- Imágenes BMP sin compresión, de **24 bits**, con ancho positivo. Se admiten orientación ascendente y descendente, cabeceras DIB de al menos 40 bytes y relleno de filas.

## Descargar

```bash
git clone https://github.com/a01735995-sketch/Transformaciones.git
cd Transformaciones
```

Los comandos siguientes se ejecutan desde la carpeta raíz del proyecto.

## Windows: compilar y ejecutar

Instala [MSYS2](https://www.msys2.org/) y Python 3.11+. En la terminal **MSYS2 UCRT64**, instala GCC:

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-libgomp
```

Abre **PowerShell** en el proyecto y agrega el compilador a la sesión si MSYS2 está en su ubicación habitual:

```powershell
$env:Path = "C:\msys64\ucrt64\bin;" + $env:Path
gcc --version
python --version
New-Item -ItemType Directory -Force build | Out-Null
gcc -std=c17 -O2 -Wall -Wextra -fopenmp bench_mark_transformaciones.c -o build\transformaciones_bench.exe
python scripts\generar_imagenes.py
python benchmark_transformaciones.py --threads 1 5 10 --repetitions 10 --warmups 1
```

También puedes compilar y medir con el archivo por lotes, después de colocar o generar las entradas:

```powershell
.\ejecutar_windows.bat --threads 1 5 10 20 40 --repetitions 10
```

Para ejecutar solamente el programa C:

```powershell
$env:OMP_NUM_THREADS = "5"
$env:OMP_DYNAMIC = "FALSE"
.\build\transformaciones_bench.exe images/input output 10
```

## Linux: compilar y ejecutar

En Debian/Ubuntu, instala GCC, Make, Git y Python. Comprueba que Python sea 3.11 o posterior:

```bash
sudo apt update
sudo apt install build-essential python3 git
python3 --version
make CC=gcc
python3 scripts/generar_imagenes.py
python3 benchmark_transformaciones.py --threads 1 5 10 --repetitions 10 --warmups 1
```

Equivalente de compilación sin Make:

```bash
mkdir -p build
gcc -std=c17 -O2 -Wall -Wextra -fopenmp bench_mark_transformaciones.c -o build/transformaciones_bench
OMP_NUM_THREADS=5 OMP_DYNAMIC=FALSE ./build/transformaciones_bench images/input output 10
```

## macOS: compilar y ejecutar

Necesitas las herramientas de desarrollo de Apple, Homebrew y Python 3.11+. La biblioteca OpenMP se instala por separado:

```bash
xcode-select --install
brew install libomp
make CC=clang
python3 scripts/generar_imagenes.py
python3 benchmark_transformaciones.py --threads 1 5 10 --repetitions 10 --warmups 1
```

Si las herramientas de Apple ya están instaladas, omite `xcode-select --install`. El Makefile consulta `brew --prefix libomp`, tanto en Apple Silicon como en Intel.

## Usar tus imágenes

Coloca `img0.bmp`, `img1.bmp`, …, `img9.bmp` en `images/input/`. Puedes usar las diez entradas de la carpeta `BMP_image-processing-library-main/images/input/` del ZIP recibido. No se suben fotografías, resultados ni ejecutables al repositorio.

El generador crea diez imágenes sintéticas de 640 × 480. Se niega a sobrescribir entradas existentes; `--force` permite reemplazarlas expresamente. Para una carga mayor o una carpeta distinta:

```bash
python3 scripts/generar_imagenes.py --count 10 --width 1920 --height 1080 --output-dir images/prueba
python3 benchmark_transformaciones.py --input-dir images/prueba --threads 1 5 10 20 40 --repetitions 10
```

En Windows sustituye `python3` por `python`. Las imágenes pequeñas sirven para comprobar el funcionamiento; sus tiempos no representan una carga grande de procesamiento.

## Qué se genera

Cada ejecución sobrescribe los archivos del mismo nombre en la carpeta de salida:

| Archivo, para la entrada `img0.bmp` | Operación |
|---|---|
| `output/Test0_HorizontalRGB.bmp` | Espejo horizontal en color |
| `output/Test0_HorizontalGray.bmp` | Espejo horizontal en gris, almacenado en 24 bits |
| `output/Test0_Rotate180RGB.bmp` | Giro de 180° en color |
| `output/Test0_Rotate180Gray.bmp` | Giro de 180° en gris, almacenado en 24 bits |

**Aclaración del código original:** `inv_img` e `inv_img_color` recorren todos los píxeles en orden inverso. Eso gira 180°; no es un reflejo vertical. Se conservan esas funciones y su comportamiento, y se corrigen los nombres de las salidas para describirlo.

## Qué mide el benchmark

Python ejecuta el programa C una vez por calentamiento y repetición. Cada proceso transforma todo el lote. Por defecto utiliza 1, 5 y 10 hilos, un calentamiento por grupo y diez mediciones por grupo.

- `wall_seconds`: tiempo externo del proceso completo, incluido arranque, validación, procesamiento y cierre.
- `c_seconds`: tiempo interno del bloque paralelo, incluidas lecturas, transformaciones, escrituras y liberación de memoria. La validación previa queda fuera de este intervalo.
- `threads_requested` y `threads_actual`: hilos solicitados y tamaño real del equipo OpenMP. Con diez imágenes hay como máximo diez iteraciones activas simultáneas, aunque se creen 20 o 40 hilos.

En `results/` se guardan `runs_*.csv`, `summary_*.csv` y `metadata_*.json`. El resumen incluye media, mínimo, máximo, desviación estándar muestral y, cuando existe un grupo de un hilo real, aceleración y eficiencia. El JSON registra sistema, ejecutable, variables OpenMP relevantes y huellas SHA-256 de las entradas.

Esta prueba mide procesamiento **y entrada/salida**. La caché del sistema, el almacenamiento, las imágenes y los hilos influyen en los tiempos. [Metodología e interpretación](https://github.com/a01735995-sketch/Transformaciones/wiki/Benchmark-y-resultados).

## Organización

```text
Transformaciones/Transformaciones.h  Funciones de transformación y lectura/escritura BMP
bench_mark_transformaciones.c       Programa C paralelo para un lote
benchmark_transformaciones.py       Calentamientos, repeticiones, estadísticas y archivos
Makefile                            Compilación en Linux/macOS y MSYS2 con Make
ejecutar_windows.bat                 Compilación y benchmark desde Windows
scripts/generar_imagenes.py          Entradas sintéticas sin dependencias
tests/                              Pruebas de píxeles, formatos, errores y medición
original/                           Código recibido, sin modificar
docs/wiki/                          Fuente de las páginas de la wiki
docs/PROCEDENCIA.md                  Origen y cambios respecto al ZIP
images/input/                       Entradas locales (no se versionan los BMP)
output/                             BMP generados (no se versionan)
results/                            CSV y JSON locales (no se versionan)
```

## Ayuda y pruebas

```bash
python3 benchmark_transformaciones.py --help
python3 scripts/generar_imagenes.py --help
./build/transformaciones_bench --help
make test
```

En Windows, después de compilar, usa `python -m unittest discover -s tests -v` y asegúrate de que `cc` o `gcc` esté en `PATH`; puedes seleccionar GCC con `$env:CC = "gcc"`. Las pruebas crean sus propias imágenes pequeñas en carpetas temporales.

Consulta la wiki para [arquitectura](https://github.com/a01735995-sketch/Transformaciones/wiki/Arquitectura), [todas las funciones](https://github.com/a01735995-sketch/Transformaciones/wiki/Transformaciones), [comparación con la otra implementación](https://github.com/a01735995-sketch/Transformaciones/wiki/Comparacion-de-implementaciones) y [solución de problemas](https://github.com/a01735995-sketch/Transformaciones/wiki/Solucion-de-problemas).
