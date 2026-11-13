# Solución de problemas

## `gcc` o `python` no se reconoce en Windows

Comprueba `gcc --version` y `python --version` en la misma terminal donde ejecutarás el proyecto. Instala GCC para UCRT64 con MSYS2 y agrega `C:\msys64\ucrt64\bin` a la sesión de PowerShell si utilizaste la ubicación habitual. Python debe ser 3.11 o posterior.

## Falta una DLL de GCC/OpenMP

Mantén el directorio `ucrt64\bin` en `PATH` mientras se ejecuta el programa. No basta con localizar `gcc` durante la compilación; Windows también debe encontrar sus bibliotecas al arrancar el ejecutable.

## `omp.h` no se encuentra en macOS

Instala `libomp` y recompila con el Makefile:

```bash
brew install libomp
make CC=clang LIBOMP_PREFIX="$(brew --prefix libomp)"
```

Apple Clang necesita las opciones del preprocesador y el enlace de `libomp`; el comando de GCC para Linux no se puede copiar sin cambios. Consulta [Instalación](https://github.com/a01735995-sketch/Transformaciones/wiki/Instalacion).

## No existe el ejecutable

Compila primero. Python busca `build/transformaciones_bench.exe` en Windows y `build/transformaciones_bench` en los demás sistemas. Puedes usar `--program` para otra ruta. El Makefile original del ZIP generaba un nombre distinto y no debe usarse para la versión actual.

## `BMP ausente, truncado o no compatible`

Comprueba que existan todas las entradas consecutivas hasta `--count - 1`, con los nombres exactos. El programa requiere BMP de 24 bits sin compresión y con dimensiones válidas. Renombrar un PNG o JPG a `.bmp` no convierte el formato.

Se admiten alturas negativas, pero no ancho negativo, altura cero, compresión, 8/16/32 bits ni un archivo que termine antes de completar sus filas. La posición de los píxeles debe estar después de la cabecera DIB.

## El generador dice que ya hay imágenes

El generador evita reemplazar tus entradas. Usa otra carpeta con `--output-dir`, o pasa `--force` si deseas sobrescribirlas. El benchmark no necesita volver a generar las imágenes entre grupos.

## No se puede usar la carpeta de salida

Verifica permisos, espacio disponible y que esa ruta sea una carpeta. La ejecución directa de C puede crear una carpeta simple, pero necesita que sus padres existan. El programa Python crea los padres cuando usas `--output-dir`.

Los fallos ocurridos durante una escritura pueden dejar salidas parciales. Corrige la causa y repite la ejecución. Cada repetición válida vuelve a escribir los mismos nombres.

## Más hilos producen un peor tiempo

Puede ocurrir por arranque y sincronización de hilos, almacenamiento saturado, caché, competencia por memoria o falta de iteraciones. Con diez imágenes, como máximo diez hilos hacen una iteración útil simultáneamente. Revisa el tamaño de las entradas, los hilos reales y la variabilidad de los CSV.

## Los hilos reales no coinciden con los solicitados

Revisa las variables `OMP_` heredadas, especialmente `OMP_THREAD_LIMIT`. Python desactiva `OMP_DYNAMIC`, pero no borra el resto del entorno. El JSON de metadatos conserva las variables relevantes y el CSV contiene ambos recuentos.

## La salida “vertical” parece girada

Ese es el comportamiento del algoritmo recibido: recorre el arreglo completo al revés. Se documenta como giro de 180° y se usa `Rotate180` en los archivos nuevos. Un reflejo vertical puro invierte solo el orden de las filas.

## Los BMP grises siguen siendo grandes

Esta implementación escribe tres canales iguales por píxel y conserva 24 bits. No produce un BMP indexado de 8 bits. Esta diferencia debe considerarse al comparar con la otra biblioteca.

## La cabecera provoca símbolos duplicados al enlazar

El código recibido define las funciones dentro de `Transformaciones.h`. Inclúyela en una sola unidad de compilación por ejecutable. Para convertirlo en una biblioteca con varios archivos C se tendría que separar declaraciones e implementación; el programa incluido no lo necesita.

## Cambié opciones de compilación y Make no recompila

Las marcas de tiempo de los archivos no detectan cambios de variables de Make. Usa `make clean` y vuelve a compilar con las opciones elegidas. `make clean` elimina el ejecutable del proyecto; conserva tus entradas y resultados.
