# Validación del proyecto

## Comprobaciones locales

Se compiló y ejecutó en macOS ARM64 con Apple Clang 21 y `libomp`. Python 3.14 ejecutó las seis pruebas de `tests/test_transformaciones.py` correctamente.

Las pruebas comprueban:

- Los píxeles completos de las cuatro operaciones del benchmark, con uno y tres hilos.
- BMP de orientación ascendente y descendente, anchos con relleno y cabeceras DIB de 40 y 124 bytes.
- Conservación de cabecera y datos posteriores a los píxeles.
- Rechazo de entradas ausentes, truncadas o incompatibles y propagación de errores de salida.
- Escala de grises, diferencias por parejas con ancho impar, histogramas y desenfoque de 9 × 9.
- Repeticiones, recuento de hilos reales, archivos CSV/JSON y huellas de entrada del benchmark Python.

También se procesaron dos imágenes grandes del ZIP recibido: `img1.bmp` (5712 × 4284, altura negativa) e `img2.bmp` (6369 × 4097, filas con relleno). Se colocaron como entradas consecutivas de una prueba de dos imágenes. Las ocho salidas tuvieron cabeceras y tamaños correctos; se comprobaron 53 posiciones de píxel por salida contra la transformación esperada. Esta comprobación muestreada complementa las comparaciones completas de las imágenes pequeñas de las pruebas.

Las cinco fuentes conservadas en `original/` se compararon byte por byte con el ZIP. Se revisaron los destinos de enlaces locales y los bloques de comandos de la documentación.

## Pruebas automáticas

El flujo [Pruebas](https://github.com/a01735995-sketch/Transformaciones/actions/workflows/pruebas.yml) compila y ejecuta las pruebas en Linux con GCC y en Windows con GCC UCRT64. Además comprueba el generador y una corrida del benchmark con imágenes sintéticas. El estado y los registros de cada ejecución se consultan en GitHub Actions.

Para repetir las pruebas localmente:

```bash
make test
```

En Windows, después de compilar y con GCC en `PATH`:

```powershell
$env:CC = "gcc"
python -m unittest discover -s tests -v
```

Estas comprobaciones verifican el funcionamiento. No constituyen un estudio de rendimiento del equipo del usuario; para ese fin utiliza varias repeticiones y conserva sus metadatos.
