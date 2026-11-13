# Instalación

## Dependencias comunes

Necesitas Git, un compilador de C17 con OpenMP y Python **3.11 o posterior**. Los programas Python usan únicamente la biblioteca estándar. No hay un archivo de dependencias que instalar con `pip`.

Clona el repositorio y entra en su raíz:

```bash
git clone https://github.com/a01735995-sketch/Transformaciones.git
cd Transformaciones
```

## Windows con GCC y PowerShell

1. Instala [MSYS2](https://www.msys2.org/).
2. Abre **MSYS2 UCRT64** e instala el compilador:

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-libgomp
```

3. Instala Python 3.11 o posterior y habilita su acceso desde `PATH`.
4. En PowerShell, dentro del proyecto, agrega las herramientas de MSYS2 a esa sesión y compila:

```powershell
$env:Path = "C:\msys64\ucrt64\bin;" + $env:Path
gcc --version
python --version
New-Item -ItemType Directory -Force build | Out-Null
gcc -std=c17 -O2 -Wall -Wextra -fopenmp bench_mark_transformaciones.c -o build\transformaciones_bench.exe
```

Si instalaste MSYS2 en otra carpeta, cambia la ruta. Mantén `ucrt64\bin` en `PATH` durante la ejecución para que Windows encuentre las bibliotecas de GCC y OpenMP. El paquete `libgomp` proporciona OpenMP y se instala expresamente porque puede ser una dependencia opcional del compilador.

Prueba rápida:

```powershell
python scripts\generar_imagenes.py
$env:OMP_NUM_THREADS = "5"
$env:OMP_DYNAMIC = "FALSE"
.\build\transformaciones_bench.exe images/input output 10
python benchmark_transformaciones.py --threads 1 5 10 --repetitions 10 --warmups 1
```

El archivo `ejecutar_windows.bat` compila y ejecuta el benchmark; reenvía sus argumentos al programa Python. No genera imágenes automáticamente:

```powershell
.\ejecutar_windows.bat --threads 1 5 10 20 40 --repetitions 10
```

## Linux con GCC

En Debian/Ubuntu:

```bash
sudo apt update
sudo apt install build-essential python3 git
python3 --version
make CC=gcc
```

Si la versión de Python de tu distribución es anterior a 3.11, utiliza una instalación más reciente. El proyecto requiere `hashlib.file_digest`, disponible desde Python 3.11.

Sin Make:

```bash
mkdir -p build
gcc -std=c17 -O2 -Wall -Wextra -fopenmp bench_mark_transformaciones.c -o build/transformaciones_bench
```

Prueba rápida:

```bash
python3 scripts/generar_imagenes.py
OMP_NUM_THREADS=5 OMP_DYNAMIC=FALSE ./build/transformaciones_bench images/input output 10
python3 benchmark_transformaciones.py --threads 1 5 10 --repetitions 10
```

## macOS con Apple Clang

Instala las herramientas de desarrollo, si aún no están disponibles, y la biblioteca OpenMP de Homebrew:

```bash
xcode-select --install
brew install libomp
make CC=clang
```

El Makefile utiliza el prefijo que devuelve `brew --prefix libomp`. Puedes indicarlo expresamente si necesitas otra instalación:

```bash
make CC=clang LIBOMP_PREFIX="$(brew --prefix libomp)"
```

Compilación equivalente sin Make:

```bash
mkdir -p build
clang -std=c17 -O2 -Wall -Wextra -Xpreprocessor -fopenmp \
  -isystem "$(brew --prefix libomp)/include" \
  bench_mark_transformaciones.c \
  -L"$(brew --prefix libomp)/lib" \
  -Wl,-rpath,"$(brew --prefix libomp)/lib" -lomp \
  -o build/transformaciones_bench
```

Prueba rápida:

```bash
python3 --version
python3 scripts/generar_imagenes.py
OMP_NUM_THREADS=5 OMP_DYNAMIC=FALSE ./build/transformaciones_bench images/input output 10
python3 benchmark_transformaciones.py --threads 1 5 10 --repetitions 10
```

## Significado de las opciones del compilador

| Opción | Finalidad |
|---|---|
| `-std=c17` | Seleccionar el estándar de C |
| `-O2` | Activar optimizaciones; mantener el mismo nivel al comparar |
| `-Wall -Wextra` | Mostrar advertencias comunes |
| `-fopenmp` | Activar y enlazar OpenMP con GCC |
| `-Xpreprocessor -fopenmp` | Activar las directivas con Apple Clang |
| `-isystem`, `-L`, `-lomp` | Localizar cabeceras y biblioteca OpenMP en macOS |
| `-Wl,-rpath,...` | Encontrar `libomp` al ejecutar en macOS |

Referencias: [entornos de MSYS2](https://www.msys2.org/docs/environments/), [libgomp para UCRT64](https://packages.msys2.org/packages/mingw-w64-ucrt-x86_64-libgomp), [libomp en Homebrew](https://formulae.brew.sh/formula/libomp).

Siguiente paso: [Uso y comandos](https://github.com/a01735995-sketch/Transformaciones/wiki/Uso-y-comandos).
