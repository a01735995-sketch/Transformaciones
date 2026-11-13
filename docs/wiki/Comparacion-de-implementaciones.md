# Comparación de las dos implementaciones del ZIP

El archivo recibido contiene `Transformaciones_OpenMP` y `BMP_image-processing-library-main`. Este repositorio extrae la primera. La segunda se describe aquí para explicar por qué los tiempos y las salidas no se pueden interpretar como si ambas hicieran exactamente lo mismo.

La comparación se basa en los archivos del **ZIP entregado**, no en posibles cambios posteriores de otros repositorios.

## Diferencias principales

| Aspecto | Transformaciones | Biblioteca BMP del ZIP |
|---|---|---|
| Programa de benchmark | `bench_mark_transformaciones.c` | `tests/bench_mark.c` |
| Reparto de trabajo | Bucle paralelo por imagen | Tarea por imagen y cuatro tareas hijas de transformación |
| Lectura de entrada | Una lectura por operación, cuatro por imagen | Una lectura por imagen; las tareas usan copias |
| Operaciones de una imagen | Cuatro funciones en secuencia dentro de su iteración | Pueden ejecutarse como tareas distintas |
| Transformación llamada vertical | Invierte el arreglo completo: giro de 180° | Invierte el orden de las filas: reflejo vertical |
| BMP gris de salida | 24 bits, tres canales iguales | 8 bits, un canal y paleta |
| Organización de funciones | Implementaciones dentro de `Transformaciones.h` | Interfaz en `include/image.h` e implementación en `src/image.c` |
| Vida de la imagen original | Se vuelve a cargar y liberar por operación | Se conserva hasta terminar las tareas hijas, con `taskwait` |

Ambas usan OpenMP y miden trabajo que incluye archivos. La estrategia de paralelismo y el volumen de datos escritos son diferentes.

## Bucle paralelo frente a tareas

En Transformaciones, diez imágenes generan diez iteraciones repartidas entre los hilos. Cada iteración ejecuta cuatro operaciones sucesivas. No hay cuarenta transformaciones independientes disponibles para el planificador.

En la biblioteca del ZIP, cada tarea padre carga una imagen y crea cuatro tareas hijas. Cada hija copia los píxeles y aplica una operación. El `taskwait` de la tarea padre mantiene válida su imagen original hasta que terminan sus hijas. OpenMP decide cómo repartir esas tareas; su orden y simultaneidad dependen del equipo y del planificador.

## Transformaciones distintas

```text
Entrada:         inv_img_color:     image_flip_vertical:
A B C            F E D             D E F
D E F            C B A             A B C
```

Que el código original use nombres `VerticalRGB` o comentarios de inversión vertical no cambia el resultado del recorrido. Esta versión denomina sus archivos `Rotate180RGB` y `Rotate180Gray` para hacer explícito ese comportamiento.

## Distinto volumen de escritura

Transformaciones escribe tres bytes de píxel tanto para color como para gris. La biblioteca escribe tres para color y uno para gris, además de su paleta. Por tanto, incluso con las mismas dimensiones y el mismo número de salidas, los bytes generados son diferentes. La fórmula de grises es comparable, pero su almacenamiento no lo es.

La diferencia afecta a memoria, escrituras y caché. Un menor tiempo por sí solo no demuestra que una estrategia paralela sea más eficiente para un trabajo equivalente.

## Qué haría falta para comparar algoritmos equivalentes

Sería necesario igualar las operaciones geométricas, el formato de salida, las entradas, la cantidad de lecturas/escrituras que se desea incluir, compilador, optimizaciones, equipo y método de medición. Después se podrían analizar ambos diseños con la misma carga.

Este proyecto conserva el giro del código recibido y documenta la diferencia. No modifica la otra biblioteca ni presenta sus tiempos como una competición equivalente.

## Los dos programas originales de Transformaciones

- `original/Transformaciones/main.c` fija cinco hilos y alterna `p1.bmp` y `p2.bmp` en diez iteraciones. Produce los nombres `inv_*`, `inv_color_*`, `espejo_*` y `espejo_color_*`.
- `original/bench_mark_transformaciones.c` usa las diez entradas `img0.bmp` a `img9.bmp` desde una carpeta hermana del ZIP, y deja que el entorno configure los hilos.
- El ejecutable actual parte de este segundo programa: admite rutas/cantidad, mantiene el reparto por imagen y permite medir los hilos desde Python.

El Makefile recibido compilaba el primer programa con rutas específicas de Homebrew; el script Python esperaba un ejecutable distinto. El Makefile y el archivo por lotes actuales compilan el mismo programa que ejecuta Python.
