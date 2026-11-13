# Uso y comandos

Todos los ejemplos parten de la raíz de `Transformaciones`. En Windows usa `python` y `build\transformaciones_bench.exe`; en Linux/macOS, `python3` y `./build/transformaciones_bench`.

## Preparar imágenes

El programa espera una secuencia sin huecos: `img0.bmp`, `img1.bmp`, …, `img{cantidad-1}.bmp`. El valor predeterminado de cantidad es diez.

Puedes copiar las diez entradas de `BMP_image-processing-library-main/images/input/` del ZIP original a `images/input/`, o generar imágenes sintéticas:

```bash
python3 scripts/generar_imagenes.py
```

El generador crea diez BMP de 640 × 480. Para otro conjunto:

```bash
python3 scripts/generar_imagenes.py --count 6 --width 1920 --height 1080 --output-dir images/prueba
```

No sobrescribe entradas existentes. `--force` permite reemplazar archivos con los mismos nombres; `--top-down` genera BMP con altura negativa. Las entradas, salidas y resultados locales quedan fuera de Git.

## Ejecutar C directamente

```text
transformaciones_bench [CARPETA_ENTRADA CARPETA_SALIDA CANTIDAD]
```

Usa los tres argumentos juntos, o ninguno. Sin argumentos se usan `images/input`, `output` y `10`. Las rutas relativas de la ejecución directa se interpretan desde la carpeta actual. El programa puede crear la carpeta de salida si su carpeta padre ya existe.

Linux/macOS:

```bash
OMP_NUM_THREADS=5 OMP_DYNAMIC=FALSE ./build/transformaciones_bench images/input output 10
```

PowerShell:

```powershell
$env:OMP_NUM_THREADS = "5"
$env:OMP_DYNAMIC = "FALSE"
.\build\transformaciones_bench.exe images/input output 10
```

El programa informa `Total time`, `OpenMP threads` e `Images`. Devuelve código 0 si todo terminó correctamente y un código distinto de cero si hubo entradas incompatibles, problemas de rutas o fallos de procesamiento/escritura.

## Ejecutar el benchmark

```bash
python3 benchmark_transformaciones.py --threads 1 5 10 20 40 --repetitions 10 --warmups 1
```

Python configura `OMP_NUM_THREADS` para cada grupo y `OMP_DYNAMIC=FALSE` para sus procesos hijos. No cambia permanentemente las variables del sistema.

| Opción | Valor predeterminado | Significado |
|---|---|---|
| `--threads` | `1 5 10` | Lista de enteros positivos, sin duplicados |
| `--repetitions` | `10` | Mediciones por grupo |
| `--warmups` | `1` | Ejecuciones previas por grupo, no incluidas en estadísticas |
| `--count` | `10` | Entradas consecutivas, entre 1 y 10000 |
| `--input-dir` | `images/input` dentro del proyecto | Ubicación de los BMP |
| `--output-dir` | `output` dentro del proyecto | Ubicación de las transformaciones |
| `--results-dir` | `results` dentro del proyecto | Ubicación de CSV y JSON |
| `--program` | Ejecutable en `build/` | Programa C compilado |
| `--timeout` | `600` | Segundos máximos por ejecución del proceso C |

Cuando pasas rutas relativas por argumentos a Python, se resuelven desde la carpeta donde lo invocas. Sus rutas predeterminadas se basan en la ubicación del script. Python crea las carpetas de salida y resultados junto con los padres necesarios.

Ejemplo con seis entradas y resultados separados:

```bash
python3 benchmark_transformaciones.py --input-dir images/prueba --count 6 \
  --output-dir output/prueba --results-dir results/prueba \
  --threads 1 2 6 --repetitions 5 --warmups 1
```

El ejemplo anterior usa continuación de línea de Bash. En PowerShell escribe el comando en una sola línea, usando `python`.

## Salidas

Cada entrada produce cuatro BMP. Los nombres contienen el índice de entrada:

```text
Test0_HorizontalGray.bmp
Test0_HorizontalRGB.bmp
Test0_Rotate180Gray.bmp
Test0_Rotate180RGB.bmp
```

Se conservan las dimensiones, la cabecera y los datos posteriores a los píxeles. El relleno de las filas se escribe con ceros. Los grises se almacenan en tres canales iguales, conservando los 24 bits del BMP.

Los calentamientos y las repeticiones vuelven a escribir las mismas salidas. No ejecutes dos benchmarks simultáneos sobre la misma carpeta de salida. Usa `--output-dir` distinto si necesitas conservar varios conjuntos.

## Ayuda y pruebas

```bash
./build/transformaciones_bench --help
python3 benchmark_transformaciones.py --help
python3 scripts/generar_imagenes.py --help
make test
```

Las pruebas verifican valores de píxeles, orientación, cabeceras, relleno, errores y archivos de resultados. No utilizan las imágenes personales del usuario.

Consulta [Benchmark y resultados](https://github.com/a01735995-sketch/Transformaciones/wiki/Benchmark-y-resultados) para interpretar las mediciones.
