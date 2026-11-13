#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include "Transformaciones/Transformaciones.h"

int main(void) {
    double inicio = omp_get_wtime();

    #pragma omp parallel for
    for (int i = 0; i < 10; i++) {
        char entrada[256];
        char salida_gris[256], salida_espejo[256];
        char salida_color[256], salida_espejo_color[256];

        sprintf(entrada, "../BMP_image-processing-library-main/images/input/img%d.bmp", i);
        sprintf(salida_gris, "output/Test%d_VerticalGray.bmp", i);
        sprintf(salida_espejo, "output/Test%d_HorizontalGray.bmp", i);
        sprintf(salida_color, "output/Test%d_VerticalRGB.bmp", i);
        sprintf(salida_espejo_color, "output/Test%d_HorizontalRGB.bmp", i);

        inv_img_grey_horizontal(salida_espejo, entrada);
        inv_img_color_horizontal(salida_espejo_color, entrada);
        inv_img(salida_gris, entrada);
        inv_img_color(salida_color, entrada);
    }

    printf("Total time: %f seconds\n", omp_get_wtime() - inicio);
    return 0;
}
