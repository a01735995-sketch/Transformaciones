#include <errno.h>
#include <limits.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#endif
#include "Transformaciones/Transformaciones.h"

#define RUTA_MAX 4096

static int ruta(char *destino, const char *carpeta, const char *nombre, int id)
{
    int longitud = snprintf(destino, RUTA_MAX, "%s/%s%d.bmp", carpeta, nombre, id);
    return longitud >= 0 && longitud < RUTA_MAX;
}

static int crear_salida(const char *carpeta)
{
#ifdef _WIN32
    int resultado = _mkdir(carpeta);
#else
    int resultado = mkdir(carpeta, 0777);
#endif
    if (resultado == 0) return 1;
    struct stat estado;
    if (errno != EEXIST || stat(carpeta, &estado) != 0) return 0;
#ifdef _WIN32
    return (estado.st_mode & _S_IFDIR) != 0;
#else
    return S_ISDIR(estado.st_mode);
#endif
}

int main(int argc, char **argv)
{
    if (argc == 2 && strcmp(argv[1], "--help") == 0) {
        puts("Uso: transformaciones_bench [CARPETA_ENTRADA CARPETA_SALIDA CANTIDAD]");
        puts("Predeterminados: images/input output 10. Entradas: img0.bmp, img1.bmp, ...");
        puts("Configura los hilos con OMP_NUM_THREADS.");
        return EXIT_SUCCESS;
    }
    if (argc != 1 && argc != 4) {
        fputs("Usa --help para consultar los argumentos.\n", stderr);
        return EXIT_FAILURE;
    }
    const char *entrada_dir = argc == 4 ? argv[1] : "images/input";
    const char *salida_dir = argc == 4 ? argv[2] : "output";
    char *fin = NULL;
    errno = 0;
    long cantidad = argc == 4 ? strtol(argv[3], &fin, 10) : 10;
    if (errno || cantidad < 1 || cantidad > 10000 ||
        (argc == 4 && (fin == argv[3] || *fin != '\0'))) {
        fputs("CANTIDAD debe ser un entero entre 1 y 10000.\n", stderr);
        return EXIT_FAILURE;
    }

    /* Validar todo el lote antes de cronometrar o escribir resultados. */
    for (int id = 0; id < cantidad; id++) {
        char entrada[RUTA_MAX];
        if (!ruta(entrada, entrada_dir, "img", id)) {
            fputs("La ruta de entrada es demasiado larga.\n", stderr);
            return EXIT_FAILURE;
        }
        FILE *archivo = fopen(entrada, "rb");
        int32_t ancho, alto;
        int valido = archivo && Leer_cabecera(archivo, NULL, &ancho, &alto);
        if (archivo) fclose(archivo);
        if (!valido) {
            fprintf(stderr, "BMP ausente, truncado o no compatible: %s\n", entrada);
            return EXIT_FAILURE;
        }
    }
    if (!crear_salida(salida_dir)) {
        fprintf(stderr, "No se puede usar la carpeta de salida: %s\n", salida_dir);
        return EXIT_FAILURE;
    }
    if (strlen(salida_dir) + 64 >= RUTA_MAX) {
        fputs("La ruta de salida es demasiado larga.\n", stderr);
        return EXIT_FAILURE;
    }

    int errores = 0, hilos_reales = 0;
    double inicio = omp_get_wtime();
    #pragma omp parallel reduction(+:errores)
    {
        #pragma omp single
        hilos_reales = omp_get_num_threads();

        #pragma omp for schedule(static)
        for (int id = 0; id < cantidad; id++) {
            char entrada[RUTA_MAX], horizontal_gris[RUTA_MAX], horizontal_rgb[RUTA_MAX];
            char giro_gris[RUTA_MAX], giro_rgb[RUTA_MAX];
            ruta(entrada, entrada_dir, "img", id);
            snprintf(horizontal_gris, sizeof(horizontal_gris), "%s/Test%d_HorizontalGray.bmp", salida_dir, id);
            snprintf(horizontal_rgb, sizeof(horizontal_rgb), "%s/Test%d_HorizontalRGB.bmp", salida_dir, id);
            snprintf(giro_gris, sizeof(giro_gris), "%s/Test%d_Rotate180Gray.bmp", salida_dir, id);
            snprintf(giro_rgb, sizeof(giro_rgb), "%s/Test%d_Rotate180RGB.bmp", salida_dir, id);
            int correcto = 1;
            correcto &= inv_img_grey_horizontal(horizontal_gris, entrada);
            correcto &= inv_img_color_horizontal(horizontal_rgb, entrada);
            correcto &= inv_img(giro_gris, entrada);
            correcto &= inv_img_color(giro_rgb, entrada);
            if (!correcto) {
                fprintf(stderr, "Fallo al transformar la imagen %d.\n", id);
                errores++;
            }
        }
    }
    printf("Total time: %.9f seconds\n", omp_get_wtime() - inicio);
    printf("OpenMP threads: %d\n", hilos_reales);
    printf("Images: %ld; outputs: %ld\n", cantidad, cantidad * 4);
    return errores ? EXIT_FAILURE : EXIT_SUCCESS;
}
