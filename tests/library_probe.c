#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Transformaciones/Transformaciones.h"

int main(int argc, char **argv)
{
    if (argc != 4) return 2;
    int correcto = 0;
    if (!strcmp(argv[1], "gray")) correcto = escala_grises(argv[3], argv[2]);
    if (!strcmp(argv[1], "blur")) correcto = desenfoque(argv[2], argv[3]);
    if (!strcmp(argv[1], "blurgray")) correcto = desenfoque_gris(argv[2], argv[3]);
    if (!strcmp(argv[1], "partial")) correcto = Parcial_x(argv[3], argv[2]);
    if (!strcmp(argv[1], "histogram")) {
        int rojo[256] = {0}, verde[256] = {0}, azul[256] = {0};
        correcto = Calcular_histograma(argv[2], rojo, verde, azul);
        long total_r = 0, total_g = 0, total_b = 0;
        for (int i = 0; i < 256; i++) {
            total_r += rojo[i]; total_g += verde[i]; total_b += azul[i];
        }
        printf("%ld %ld %ld\n", total_r, total_g, total_b);
    }
    if (!strcmp(argv[1], "emptyhist")) {
        int cero[256] = {0};
        Graficar_histograma(cero, cero, cero);
        correcto = 1;
    }
    return correcto ? 0 : 1;
}
