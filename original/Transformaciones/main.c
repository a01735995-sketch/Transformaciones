#include <stdio.h>
#include <stdlib.h>
#include "omp.h"
#include "Transformaciones.h"

#define NUM_THREADS 5

int main(void)
{
    // Declarar Numero de threads
    omp_set_num_threads(NUM_THREADS);

    double inicio = omp_get_wtime();

    #pragma omp parallel for
    for (int i = 1; i <= 10; i++)
    {
        // Declarar arreglos de imagenes a leer y a guardar
        char entrada[32];
        char salida_gris[32], salida_espejo[32];
        char salida_color[32], salida_espejo_color[32];
        // Fin declaracion
        

        sprintf(entrada, "p%d.bmp", (i - 1) % 2 + 1);
        sprintf(salida_gris, "inv_%d.bmp", i);
        sprintf(salida_espejo, "espejo_%d.bmp", i);
        sprintf(salida_color, "inv_color_%d.bmp", i);
        sprintf(salida_espejo_color, "espejo_color_%d.bmp", i);

        inv_img(salida_gris, entrada);
        inv_img_grey_horizontal(salida_espejo, entrada);
        inv_img_color(salida_color, entrada);
        inv_img_color_horizontal(salida_espejo_color, entrada);
    }
    printf("Tiempo total: %.3f segundos\n", omp_get_wtime() - inicio);
    return 0;
}
