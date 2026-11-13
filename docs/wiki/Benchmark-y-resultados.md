# Benchmark y resultados

## Unidad de trabajo

Una ejecución del programa C procesa todo el lote. Para `N` imágenes, genera `4N` salidas. Las cuatro operaciones son espejo horizontal y giro de 180°, cada una en color y gris. No se incluyen desenfoques, histogramas ni `Parcial_x`.

Python lanza un proceso nuevo para cada calentamiento y repetición. Por ejemplo:

```bash
python3 benchmark_transformaciones.py --threads 1 5 10 20 40 --repetitions 10 --warmups 1
```

Este comando realiza 55 ejecuciones completas: cinco grupos, cada uno con un calentamiento y diez mediciones. Los diez valores de cada grupo son observaciones medidas; los calentamientos se excluyen.

## Dos intervalos de tiempo

| Campo | Intervalo |
|---|---|
| `wall_seconds` | Python usa `time.perf_counter()` alrededor del proceso hijo: lanzamiento, validación, procesamiento, salida del programa y recogida de su salida |
| `c_seconds` | C usa `omp_get_wtime()` alrededor de la región paralela: creación del equipo, rutas por imagen, lecturas, conversiones, escrituras, cierres, liberación de búferes y sincronización |

La validación de las cabeceras de todas las entradas y la preparación de la carpeta de salida ocurren antes del cronómetro interno. Ambos intervalos incluyen trabajo de archivos; ninguno es una medición aislada de operaciones de CPU. El guardado de los CSV/JSON y el cálculo previo de huellas de entrada no forman parte de las repeticiones cronometradas.

El cierre de un archivo no obliga a sincronizar físicamente el disco con `fsync`. Los tiempos reflejan las escrituras y la caché normales del sistema operativo, no la durabilidad de los datos en el dispositivo.

## Hilos solicitados y reales

Python establece `OMP_NUM_THREADS` y desactiva el ajuste dinámico con `OMP_DYNAMIC=FALSE`. El programa C consulta `omp_get_num_threads()` dentro de la región paralela y devuelve el tamaño real del equipo.

Las otras variables `OMP_` heredadas se registran en los metadatos. Por ejemplo, `OMP_THREAD_LIMIT` puede limitar el equipo aunque se soliciten más hilos. Un equipo de cuarenta hilos puede existir con solo diez iteraciones trabajando; `threads_actual` cuenta el equipo completo, incluidos los hilos sin una imagen asignada.

## Archivos de resultados

Cada corrida completa genera tres archivos con la misma marca temporal UTC:

```text
results/runs_<marca>.csv
results/summary_<marca>.csv
results/metadata_<marca>.json
```

### Mediciones: `runs_*.csv`

| Columna | Significado |
|---|---|
| `threads_requested` | Hilos configurados para ese grupo |
| `threads_actual` | Tamaño real del equipo OpenMP |
| `run` | Repetición dentro del grupo, empezando en 1 |
| `wall_seconds` | Tiempo externo en segundos |
| `c_seconds` | Tiempo interno en segundos |

### Estadísticas: `summary_*.csv`

Contiene una fila por grupo con hilos solicitados/reales, número de repeticiones, media, mínimo, máximo y desviación estándar muestral del tiempo externo; también incluye la media del tiempo interno.

La desviación estándar utiliza `n - 1` en el denominador. Si solo hay una repetición, se guarda cero; ese valor no permite estimar la variabilidad de la prueba.

Si se midió un grupo que solicitó y obtuvo un hilo, se calcula:

```text
aceleración = media_del_tiempo_externo_con_1_hilo / media_del_grupo
eficiencia = aceleración / hilos_reales_del_grupo
```

`speedup_vs_1` y `efficiency` quedan vacíos cuando no existe esa referencia. La eficiencia se expresa como fracción: para presentarla en porcentaje, multiplícala por cien.

Una aceleración mayor que 1 significa menor tiempo que la referencia. Menor que 1 significa que el grupo tardó más. No presupongas que más hilos mejoran el resultado.

### Contexto: `metadata_*.json`

Registra fecha de inicio UTC, sistema, arquitectura, procesador cuando la plataforma lo informa, CPU lógicas, Python, rutas, configuración, variables OpenMP heredadas y huellas SHA-256 del ejecutable, la cabecera de transformaciones y cada entrada. Incluye nombres y tamaños de los BMP.

El JSON no identifica automáticamente la versión ni los argumentos del compilador. Para un informe reproducible, anota además la salida de `gcc --version` o `clang --version` y el comando de compilación utilizado.

## Cómo hacer una comparación útil

1. Usa exactamente las mismas entradas, cantidad y transformaciones en todos los grupos.
2. Mantén el compilador, las optimizaciones, el almacenamiento y la configuración de energía.
3. Ejecuta en un equipo con carga estable y registra sus características.
4. Incluye un grupo de un hilo y varias repeticiones por grupo.
5. Repite la secuencia completa si necesitas comprobar estabilidad u orden de ejecución.
6. Conserva los CSV y el JSON juntos y distingue los tiempos externos de los internos al presentar tablas.

Los grupos se ejecutan en el orden indicado; no se aleatorizan. Leer las imágenes para calcular sus huellas y realizar calentamientos puede llenar la caché antes de las mediciones. Esta metodología describe ejecuciones con caché normal o caliente; no una prueba garantizada de disco frío.

## Interpretación y límites

- Con diez imágenes hay como máximo diez iteraciones activas. Probar veinte o cuarenta hilos permite observar sobrecostos, pero no aumenta la cantidad de trabajo disponible.
- La lectura repetida y las escrituras pueden dominar el tiempo. El almacenamiento y la presión de memoria importan junto con el procesador.
- Las imágenes sintéticas pequeñas comprueban el flujo; pueden producir tiempos dominados por arranque y administración de hilos.
- El benchmark detiene la corrida si falla un proceso. No guarda un resumen final de todas las configuraciones si la corrida quedó incompleta.
- Las salidas se sobrescriben en cada repetición; no representan archivos independientes acumulados por cada medición.

No se incluyen cifras de rendimiento universales en esta wiki. Para medir tu equipo, ejecuta los comandos y consulta tus archivos locales. Antes de comparar las dos implementaciones del ZIP, revisa sus [diferencias de trabajo y formato](https://github.com/a01735995-sketch/Transformaciones/wiki/Comparacion-de-implementaciones).
