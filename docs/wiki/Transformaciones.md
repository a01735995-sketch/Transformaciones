# Funciones de transformación

El archivo `Transformaciones/Transformaciones.h` contiene las funciones del proyecto. La copia recibida está en `original/Transformaciones/Transformaciones.h`.

## Funciones de imagen

Las funciones siguientes devuelven **1 si terminan correctamente y 0 si fallan**. Los argumentos cambian de orden en los desenfoques por compatibilidad con el código recibido; consúltalos antes de usarlas.

| Función | Orden de argumentos | Comportamiento | Se mide por defecto |
|---|---|---|---|
| `escala_grises` | salida, entrada | Convierte a gris sin cambiar posiciones | No |
| `inv_img_grey_horizontal` | salida, entrada | Espejo horizontal y conversión a gris | Sí |
| `inv_img_color_horizontal` | salida, entrada | Espejo horizontal en color | Sí |
| `inv_img` | salida, entrada | Giro de 180° y conversión a gris | Sí |
| `inv_img_color` | salida, entrada | Giro de 180° en color | Sí |
| `desenfoque` | entrada, salida | Promedio espacial de 9 × 9 en color | No |
| `desenfoque_gris` | entrada, salida | Conversión a gris y promedio de 9 × 9 | No |
| `Parcial_x` | salida, entrada | Diferencias entre parejas de píxeles de una fila | No |

## Escala de grises

La intensidad se calcula como:

```text
gris = truncar(0.21 × R + 0.72 × G + 0.07 × B)
```

El resultado se copia en los tres canales del píxel. Por eso los BMP grises generados continúan siendo de **24 bits**. No se crea una paleta de grises ni se reduce el archivo a un byte por píxel.

## Espejo horizontal

Para una imagen de ancho `W`, el píxel de destino `(x, y)` toma el valor de `(W - 1 - x, y)`. Se invierte el orden de las columnas de cada fila.

```text
Entrada:       Espejo horizontal:
A B C          C B A
D E F          F E D
```

## Giro de 180° y nombres históricos

`inv_img` e `inv_img_color` recorren el arreglo completo desde el último píxel al primero. El resultado invierte simultáneamente filas y columnas:

```text
Entrada:       Giro de 180°:       Reflejo vertical puro:
A B C          F E D              D E F
D E F          C B A              A B C
```

La última columna ilustra una operación diferente, que estas funciones no implementan. Se mantienen los nombres de función originales para facilitar la comparación con el ZIP. El ejecutable adaptado usa `Rotate180` en los nombres de salida, en lugar de `Vertical`.

## Desenfoque

El kernel contiene 81 coeficientes de `1/81`. Se promedian los canales de cada vecindad de 9 × 9 y se redondea sumando `0.5` antes de convertir a byte. Solo se escriben nuevos valores en las posiciones con vecindad completa; el borde de cuatro píxeles conserva su valor previo.

Si la imagen es menor que el kernel en alguna dimensión, se informa la situación y se guarda sin desenfoque. En `desenfoque_gris` esa imagen ya está convertida a gris. Estas funciones no se ejecutan en el benchmark predeterminado.

## Diferencias por parejas

`Parcial_x` calcula, para cada pareja de una fila, `segundo - primero` en cada canal, almacena la diferencia en el primero y pone el segundo en cero. La resta se convierte a byte: los valores negativos se representan módulo 256. No es una derivada con signo ni un detector de bordes normalizado.

La versión adaptada respeta los límites de cada fila y conserva un último píxel si el ancho es impar. El original podía acceder fuera del arreglo con cantidades impares y colocaba el relleno de filas de forma incorrecta.

## Histogramas

`Calcular_histograma(entrada, H_R, H_G, H_B)` acumula frecuencias por canal y devuelve 1 o 0. Pasa tres arreglos de 256 enteros **inicializados en cero**. La función suma sobre su contenido previo.

`Graficar_histograma(H_R, H_G, H_B)` conserva el visualizador de consola original: agrupa intensidades en 32 intervalos y dibuja el canal rojo, con altura de 20 líneas. Aunque recibe tres canales, no imprime tres gráficas. La versión adaptada evita dividir entre cero si el histograma está vacío.

## Ejemplo de uso de la cabecera

```c
#include "Transformaciones/Transformaciones.h"

int main(void) {
    if (!inv_img_color_horizontal("espejo.bmp", "entrada.bmp")) {
        return 1;
    }
    if (!desenfoque("entrada.bmp", "desenfoque.bmp")) {
        return 1;
    }
    return 0;
}
```

Incluye esta cabecera en una sola unidad de compilación del ejecutable y utiliza archivos distintos para entrada y salida. Sus operaciones de píxeles son secuenciales; OpenMP se utiliza en el programa que distribuye las imágenes.
