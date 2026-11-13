# Procedencia y cambios

El código recibido proviene de `Benchmarks_BMP_Windows.zip`, carpeta `Benchmarks_BMP_Windows/Transformaciones_OpenMP/`. Se extrajeron las fuentes, los dos programas de ejecución, el script de medición y el Makefile. Las imágenes, los ejecutables, los resultados y los datos personales del proyecto de Xcode no se incorporaron a este repositorio.

La biblioteca `BMP_image-processing-library-main` incluida en el mismo ZIP no se copió. Su código se consultó para documentar las diferencias entre implementaciones.

## Copia exacta recibida

Los siguientes archivos se conservan sin cambios bajo `original/`. Sus huellas permiten comprobar la correspondencia con el ZIP:

| Archivo | SHA-256 |
|---|---|
| `original/Makefile` | `b5db2f19728b88b076fb664f093b7edc8f3e674880a525999d9838844b700516` |
| `original/Transformaciones/Transformaciones.h` | `3b2fa996454571668727aa87c83f12fd7a9a6117f89291ef5acdedd86d9f2284` |
| `original/Transformaciones/main.c` | `2c31d510ccf3bb3b42baeea15590e455ecfe4f711da46710faa480050350f0d8` |
| `original/bench_mark_transformaciones.c` | `7616b119c04e845fc83dfd3e514db6553b75d622ad9088345c545a7a3cd9c404` |
| `original/benchmark_transformaciones.py` | `c5d77191e1b30b9bbdad97034990831fdcee4b8386ef2761371b5d3ce7869680` |

`original/Transformaciones/main.c` alterna `p1.bmp` y `p2.bmp` en diez iteraciones con cinco hilos fijos. `original/bench_mark_transformaciones.c` utiliza las diez imágenes `img0.bmp` a `img9.bmp` de una carpeta hermana del ZIP. Estos archivos se preservan como referencia; los comandos del README ejecutan la versión adaptada de la raíz.

## Adaptaciones para ejecutar el proyecto por separado

- Se eliminaron las dependencias de rutas a una carpeta hermana. El programa C admite directorio de entrada, directorio de salida y cantidad.
- Se unificó el nombre del ejecutable que compilan Make y Windows y el que ejecuta Python.
- Se mantiene el reparto paralelo por imagen. Una región `parallel` con un `for` estático permite registrar el tamaño real del equipo. No se usa el número fijo de hilos del primer ejemplo.
- Se verifican formatos, dimensiones, tamaños y operaciones de archivo; también se comprueban reservas de memoria y se propagan errores con retornos 1/0.
- Las alturas BMP negativas se normalizan para los recorridos, conservando la orientación de la cabecera de salida. Esto permite usar la entrada `img1.bmp` del ZIP, que tiene altura negativa.
- Se conserva la inversión completa del arreglo de `inv_img` e `inv_img_color`: gira 180°. Se corrigieron comentarios y nombres de salida a `Rotate180`, sin sustituirla por un reflejo vertical.
- El kernel de desenfoque es constante. Se corrigieron los límites y el relleno por filas de `Parcial_x`; un píxel final impar se conserva. El histograma vacío evita división entre cero.
- Python acepta hilos, repeticiones, calentamientos y rutas por argumentos; guarda ambos tiempos, estadísticas, aceleración, eficiencia y metadatos con huellas de las entradas.
- Se añadieron un generador sintético, pruebas de comportamiento y documentación en español. No se añadieron paquetes de Python.

## Comportamiento conservado

Los grises siguen almacenándose en tres canales de 24 bits. Las cuatro operaciones del benchmark vuelven a leer cada imagen por separado y se ejecutan en secuencia dentro de cada iteración. Los desenfoques y histogramas están disponibles en la cabecera, pero no se incluyen en el benchmark predeterminado. La cabecera mantiene el estilo de implementación en un único archivo; no se convirtió en la API de la otra biblioteca.
