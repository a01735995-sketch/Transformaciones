# Transformaciones BMP con OpenMP

Este proyecto aplica transformaciones a imágenes BMP mediante C y utiliza Python para medir ejecuciones completas del programa. OpenMP distribuye las imágenes entre hilos; dentro de cada imagen, las cuatro operaciones se ejecutan en secuencia.

Con diez entradas se producen cuarenta BMP por ejecución. El objetivo es estudiar cómo cambia el tiempo al variar los hilos, considerando también el costo de leer y escribir archivos.

## Guías

| Página | Contenido |
|---|---|
| [Instalación](https://github.com/a01735995-sketch/Transformaciones/wiki/Instalacion) | Dependencias y compilación en Windows, Linux y macOS |
| [Uso y comandos](https://github.com/a01735995-sketch/Transformaciones/wiki/Uso-y-comandos) | Entradas, ejecución directa, opciones de Python y salidas |
| [Arquitectura](https://github.com/a01735995-sketch/Transformaciones/wiki/Arquitectura) | Flujo de datos, memoria y paralelismo |
| [Transformaciones](https://github.com/a01735995-sketch/Transformaciones/wiki/Transformaciones) | Funciones, fórmulas y comportamiento real |
| [Benchmark y resultados](https://github.com/a01735995-sketch/Transformaciones/wiki/Benchmark-y-resultados) | Tiempos, estadísticas, CSV, JSON y metodología |
| [Comparación de implementaciones](https://github.com/a01735995-sketch/Transformaciones/wiki/Comparacion-de-implementaciones) | Diferencias frente a la otra biblioteca del ZIP |
| [Solución de problemas](https://github.com/a01735995-sketch/Transformaciones/wiki/Solucion-de-problemas) | Compilación, formatos, rutas y rendimiento |

## Primera prueba

Después de [instalar el compilador](https://github.com/a01735995-sketch/Transformaciones/wiki/Instalacion), ejecuta desde la raíz del repositorio, en Linux/macOS:

```bash
make
python3 scripts/generar_imagenes.py
python3 benchmark_transformaciones.py --threads 1 5 10 --repetitions 10 --warmups 1
```

En PowerShell, con GCC y Python en `PATH`:

```powershell
python scripts\generar_imagenes.py
.\ejecutar_windows.bat --threads 1 5 10 --repetitions 10 --warmups 1
```

## Procedencia

La fuente es la carpeta `Transformaciones_OpenMP` de `Benchmarks_BMP_Windows.zip`. Se conserva una copia exacta en [`original/`](https://github.com/a01735995-sketch/Transformaciones/tree/main/original). La versión de trabajo mantiene los algoritmos y añade correcciones de lectura BMP, comprobaciones de errores y una ejecución configurable. El detalle está en [Procedencia](https://github.com/a01735995-sketch/Transformaciones/blob/main/docs/PROCEDENCIA.md).

Las funciones históricas `inv_img` e `inv_img_color` realizan un **giro de 180°**. Los nombres originales que aluden a inversión vertical no describen un reflejo vertical puro. Esta diferencia es relevante al comparar resultados.
