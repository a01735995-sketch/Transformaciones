#ifndef TRANSFORMACIONES_H
#define TRANSFORMACIONES_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ALTURA 20
#define ANCHO 32
#define n 9
#define m 9

typedef struct {
    unsigned char b;
    unsigned char g;
    unsigned char r;
} Pixel;

typedef struct {
    unsigned char IG1;
    unsigned char IG2;
    unsigned char IG3;
} Pixel_gris;

float Kernel[n][m] = {
    {1.0f/81, 1.0f/81, 1.0f/81, 1.0f/81, 1.0f/81, 1.0f/81, 1.0f/81, 1.0f/81, 1.0f/81},
    {1.0f/81, 1.0f/81, 1.0f/81, 1.0f/81, 1.0f/81, 1.0f/81, 1.0f/81, 1.0f/81, 1.0f/81},
    {1.0f/81, 1.0f/81, 1.0f/81, 1.0f/81, 1.0f/81, 1.0f/81, 1.0f/81, 1.0f/81, 1.0f/81},
    {1.0f/81, 1.0f/81, 1.0f/81, 1.0f/81, 1.0f/81, 1.0f/81, 1.0f/81, 1.0f/81, 1.0f/81},
    {1.0f/81, 1.0f/81, 1.0f/81, 1.0f/81, 1.0f/81, 1.0f/81, 1.0f/81, 1.0f/81, 1.0f/81},
    {1.0f/81, 1.0f/81, 1.0f/81, 1.0f/81, 1.0f/81, 1.0f/81, 1.0f/81, 1.0f/81, 1.0f/81},
    {1.0f/81, 1.0f/81, 1.0f/81, 1.0f/81, 1.0f/81, 1.0f/81, 1.0f/81, 1.0f/81, 1.0f/81},
    {1.0f/81, 1.0f/81, 1.0f/81, 1.0f/81, 1.0f/81, 1.0f/81, 1.0f/81, 1.0f/81, 1.0f/81},
    {1.0f/81, 1.0f/81, 1.0f/81, 1.0f/81, 1.0f/81, 1.0f/81, 1.0f/81, 1.0f/81, 1.0f/81}
};

void Leer_cabecera(FILE *image, FILE *outputImage, int32_t *ancho, int32_t *alto)
{
    uint32_t inicio_pixeles;
    fseek(image, 10, SEEK_SET);
    fread(&inicio_pixeles, sizeof(inicio_pixeles), 1, image);
    // Buscar ancho y alto en los primeros 18 bytes
    fseek(image, 18, SEEK_SET);
    fread(ancho, sizeof(*ancho), 1, image);
    fread(alto, sizeof(*alto), 1, image);
    // Fin de busqueda
    // Declarar arreglos dinamicos y reiniciar lectura a inicio de imagen
    fseek(image, 0, SEEK_SET);
    // Lectura de cabecero imagen
    for (uint32_t j = 0; j < inicio_pixeles; j++)
    {
        int dato = fgetc(image);
        fputc(dato, outputImage);
    }
    // Fin de Lectura
}

Pixel *Leer_pixeles(FILE *image, int32_t ancho, int32_t alto)
{
    // Declarar arreglos dinamicos y reiniciar lectura a inicio de imagen
    Pixel *pixeles = malloc(ancho * alto * sizeof(Pixel));
    int relleno = (4 - (ancho * 3) % 4) % 4;
    for (int fila = 0; fila < alto; fila++)
    {
        // Lectura de bytes para cada pixel
        fread(&pixeles[fila * ancho], sizeof(Pixel), ancho, image);
        fseek(image, relleno, SEEK_CUR);
        // Fin
    }
    return pixeles;
}

void Escribir_relleno(FILE *outputImage, int32_t ancho)
{
    unsigned char relleno[3] = {0};
    fwrite(relleno, 1, (4 - (ancho * 3) % 4) % 4, outputImage);
}

void Copiar_final(FILE *image, FILE *outputImage)
{
    unsigned char bloque[4096];
    size_t cantidad;
    while ((cantidad = fread(bloque, 1, sizeof(bloque), image)) > 0)
        fwrite(bloque, 1, cantidad, outputImage);
}

Pixel_gris *Convertir_grises(Pixel *pixeles, int32_t ancho, int32_t alto)
{
    Pixel_gris *pixeles_gris = malloc(ancho * alto * sizeof(Pixel_gris));
    for (int i = 0; i < ancho * alto; i++)
    {
        // Creacion de estructuras
        Pixel p = pixeles[i];
        // Fin
        // Transformacion escala de grises y guardar estructura
        unsigned char pixel_gris = 0.21*p.r + 0.72*p.g + 0.07*p.b;
        pixeles_gris[i].IG1 = pixeles_gris[i].IG2 = pixeles_gris[i].IG3 = pixel_gris;
        // Fin_modificacion
    }
    return pixeles_gris;
}

void escala_grises(const char *salida, const char *entrada)
{
    // Declarar arreglos de imagenes a leer y a guardar
    FILE *image = fopen(entrada, "rb");
    FILE *outputImage = fopen(salida, "wb");
    // Fin declaracion
    int32_t ancho, alto;
    Leer_cabecera(image, outputImage, &ancho, &alto);
    Pixel *pixeles = Leer_pixeles(image, ancho, alto);
    Pixel_gris *pixeles_gris = Convertir_grises(pixeles, ancho, alto);
    for (int i = 0; i < ancho * alto; i++)
    {
        fwrite(&pixeles_gris[i], sizeof(Pixel_gris), 1, outputImage);
        if ((i + 1) % ancho == 0)
            Escribir_relleno(outputImage, ancho);
    }
    Copiar_final(image, outputImage);
    // Funciones de Cierre
    fclose(image);
    fclose(outputImage);
    free(pixeles);
    free(pixeles_gris);
}

void inv_img(const char *salida, const char *entrada)
{
    // Declarar arreglos de imagenes a leer y a guardar
    FILE *image = fopen(entrada, "rb");
    FILE *outputImage = fopen(salida, "wb");
    // Fin declaracion
    int32_t ancho, alto;
    Leer_cabecera(image, outputImage, &ancho, &alto);
    Pixel *pixeles = Leer_pixeles(image, ancho, alto);
    Pixel_gris *pixeles_gris = Convertir_grises(pixeles, ancho, alto);
    // Impresion de arreglos e inversiones verticales
    for (int x = ancho * alto - 1; x >= 0; x--)
    {
        fwrite(&pixeles_gris[x], sizeof(Pixel_gris), 1, outputImage);
        if (x % ancho == 0)
            Escribir_relleno(outputImage, ancho);
    }
    // Fin de inversion vertical
    Copiar_final(image, outputImage);
    // Funciones de Cierre
    fclose(image);
    fclose(outputImage);
    free(pixeles);
    free(pixeles_gris);
}

void Parcial_x(const char *salida, const char *entrada) {
    // Declarar arreglos de imagenes a leer y a guardar
    FILE *image = fopen(entrada, "rb");
    FILE *outputImage = fopen(salida, "wb");
    int Resta = 0;
    // Fin declaracion
    int32_t ancho, alto;
    Leer_cabecera(image, outputImage, &ancho, &alto);
    Pixel *pixeles = Leer_pixeles(image, ancho, alto);
    // Impresion derivada parcial
    for (int x = 0; x <= ancho * alto - 1; x+=2)
    {
        pixeles[x].b = pixeles[x + 1].b - pixeles[x].b;
        pixeles[x].r = pixeles[x + 1].r - pixeles[x].r;
        pixeles[x].g = pixeles[x + 1].g - pixeles[x].g;
        pixeles[x + 1].b = 0;
        pixeles[x + 1].r = 0;
        pixeles[x + 1].g = 0;
        fwrite(&pixeles[x], sizeof(Pixel), 1, outputImage);
        fwrite(&pixeles[x + 1], sizeof(Pixel), 1, outputImage);
        if (x % ancho == 0)
            Escribir_relleno(outputImage, ancho);
    }
    Copiar_final(image, outputImage);
    // Funciones de Cierre
    fclose(image);
    fclose(outputImage);
    free(pixeles);
    
}

void inv_img_color(const char *salida, const char *entrada)
{
    // Declarar arreglos de imagenes a leer y a guardar
    FILE *image = fopen(entrada, "rb");
    FILE *outputImage = fopen(salida, "wb");
    // Fin declaracion
    int32_t ancho, alto;
    Leer_cabecera(image, outputImage, &ancho, &alto);
    Pixel *pixeles = Leer_pixeles(image, ancho, alto);
    // Impresion de arreglos e inversiones verticales
    for (int x = ancho * alto - 1; x >= 0; x--)
    {
        fwrite(&pixeles[x], sizeof(Pixel), 1, outputImage);
        if (x % ancho == 0)
            Escribir_relleno(outputImage, ancho);
    }
    // Fin de inversion vertical
    Copiar_final(image, outputImage);
    // Funciones de Cierre
    fclose(image);
    fclose(outputImage);
    free(pixeles);
}

void inv_img_grey_horizontal(const char *salida, const char *entrada)
{
    // Declarar arreglos de imagenes a leer y a guardar
    FILE *image = fopen(entrada, "rb");
    FILE *outputImage = fopen(salida, "wb");
    // Fin declaracion
    int32_t ancho, alto;
    Leer_cabecera(image, outputImage, &ancho, &alto);
    Pixel *pixeles = Leer_pixeles(image, ancho, alto);
    Pixel_gris *pixeles_gris = Convertir_grises(pixeles, ancho, alto);
    // Inversion horizontal de imagen a color y escala de grises
    for (int fila = 0; fila < alto; fila++)
    {
        for (int columna = ancho - 1; columna >= 0; columna--)
        {
            int posicion = fila * ancho + columna;
            fwrite(&pixeles_gris[posicion], sizeof(Pixel_gris), 1, outputImage);
        }
        Escribir_relleno(outputImage, ancho);
    }
    // Fin de inversion horizontal
    Copiar_final(image, outputImage);
    // Funciones de Cierre
    fclose(image);
    fclose(outputImage);
    free(pixeles);
    free(pixeles_gris);
}

void inv_img_color_horizontal(const char *salida, const char *entrada)
{
    // Declarar arreglos de imagenes a leer y a guardar
    FILE *image = fopen(entrada, "rb");
    FILE *outputImage = fopen(salida, "wb");
    // Fin declaracion
    int32_t ancho, alto;
    Leer_cabecera(image, outputImage, &ancho, &alto);
    Pixel *pixeles = Leer_pixeles(image, ancho, alto);
    // Inversion horizontal de imagen a color y escala de grises
    for (int fila = 0; fila < alto; fila++)
    {
        for (int columna = ancho - 1; columna >= 0; columna--)
        {
            int posicion = fila * ancho + columna;
            fwrite(&pixeles[posicion], sizeof(Pixel), 1, outputImage);
        }
        Escribir_relleno(outputImage, ancho);
    }
    // Fin de inversion horizontal
    Copiar_final(image, outputImage);
    // Funciones de Cierre
    fclose(image);
    fclose(outputImage);
    free(pixeles);
}

void desenfoque(const char *entrada, const char *salida)
{
    // Declarar arreglos de imagenes a leer y a guardar
    FILE *image = fopen(entrada, "rb");
    FILE *outputImage = fopen(salida, "wb");
    // Fin declaracion
    int32_t ancho, alto;
    Leer_cabecera(image, outputImage, &ancho, &alto);
    Pixel *pixeles = Leer_pixeles(image, ancho, alto);
    Pixel *pixeles_blur = malloc(ancho * alto * sizeof(Pixel));
    //guardar arreglos de blurr
    for (int k = 0; k < ancho * alto; k++)
        pixeles_blur[k] = pixeles[k];
    //Fin de guardado

    // Filtrado con kernel para las dos imagenes
    int cont_f = 0, cont_c = 0;
    int Kernel_C = n, Kernel_F = m;
    if (alto < n || ancho < m)
        printf("El kernel es mas grande que la imagen\n");
    else
    {
        while (Kernel_C <= alto)
        {
            while (Kernel_F <= ancho)
            {
                float suma_r = 0, suma_g = 0, suma_b = 0;
                for (int x = 0; x < n; x++)
                {
                    for (int i = 0; i < m; i++)
                    {
                        int posicion = (x + cont_c) * ancho + (i + cont_f);
                        // Blur RGB
                        suma_r += pixeles[posicion].r * Kernel[x][i];
                        suma_g += pixeles[posicion].g * Kernel[x][i];
                        suma_b += pixeles[posicion].b * Kernel[x][i];
                    }
                }
                int posicion = (cont_c + n/2) * ancho + (cont_f + m/2);
                // Guardar pixel RGB
                pixeles_blur[posicion].r = (unsigned char)(suma_r + 0.5f);
                pixeles_blur[posicion].g = (unsigned char)(suma_g + 0.5f);
                pixeles_blur[posicion].b = (unsigned char)(suma_b + 0.5f);
                // Desplazamiento horizontal
                cont_f++;
                Kernel_F++;
            }
            // Desplazamiento vertical
            cont_f = 0;
            Kernel_F = m;
            cont_c++;
            Kernel_C++;
        }
    }
    // Fin de blurr para imagenes
    for (int k = 0; k < ancho * alto; k++)
    {
        fwrite(&pixeles_blur[k], sizeof(Pixel), 1, outputImage);
        if ((k + 1) % ancho == 0)
            Escribir_relleno(outputImage, ancho);
    }
    Copiar_final(image, outputImage);
    // Funciones de Cierre
    fclose(image);
    fclose(outputImage);
    free(pixeles);
    free(pixeles_blur);
}

void desenfoque_gris(const char *entrada, const char *salida)
{
    // Declarar arreglos de imagenes a leer y a guardar
    FILE *image = fopen(entrada, "rb");
    FILE *outputImage = fopen(salida, "wb");
    // Fin declaracion
    int32_t ancho, alto;
    Leer_cabecera(image, outputImage, &ancho, &alto);
    Pixel *pixeles = Leer_pixeles(image, ancho, alto);
    Pixel_gris *pixeles_gris = Convertir_grises(pixeles, ancho, alto);
    Pixel_gris *pixeles_gris_blur = malloc(ancho * alto * sizeof(Pixel_gris));
    //guardar arreglos de blurr
    for (int k = 0; k < ancho * alto; k++)
        pixeles_gris_blur[k] = pixeles_gris[k];
    //Fin de guardado

    // Filtrado con kernel para las dos imagenes
    int cont_f = 0, cont_c = 0;
    int Kernel_C = n, Kernel_F = m;
    if (alto < n || ancho < m)
        printf("El kernel es mas grande que la imagen\n");
    else
    {
        while (Kernel_C <= alto)
        {
            while (Kernel_F <= ancho)
            {
                float suma_gris = 0;
                for (int x = 0; x < n; x++)
                {
                    for (int i = 0; i < m; i++)
                    {
                        int posicion = (x + cont_c) * ancho + (i + cont_f);
                        // Blur escala de grises
                        suma_gris += pixeles_gris[posicion].IG1 * Kernel[x][i];
                    }
                }
                int posicion = (cont_c + n/2) * ancho + (cont_f + m/2);
                // Guardar pixel gris
                unsigned char gris = (unsigned char)(suma_gris + 0.5f);
                pixeles_gris_blur[posicion].IG1 = gris;
                pixeles_gris_blur[posicion].IG2 = gris;
                pixeles_gris_blur[posicion].IG3 = gris;
                // Desplazamiento horizontal
                cont_f++;
                Kernel_F++;
            }
            // Desplazamiento vertical
            cont_f = 0;
            Kernel_F = m;
            cont_c++;
            Kernel_C++;
        }
    }
    // Fin de blurr para imagenes
    for (int k = 0; k < ancho * alto; k++)
    {
        fwrite(&pixeles_gris_blur[k], sizeof(Pixel_gris), 1, outputImage);
        if ((k + 1) % ancho == 0)
            Escribir_relleno(outputImage, ancho);
    }
    Copiar_final(image, outputImage);
    // Funciones de Cierre
    fclose(image);
    fclose(outputImage);
    free(pixeles);
    free(pixeles_gris);
    free(pixeles_gris_blur);
}

void Calcular_histograma(const char *entrada, int H_R[], int H_G[], int H_B[])
{
    FILE *image = fopen(entrada, "rb");
    int32_t ancho, alto;
    uint32_t inicio_pixeles;
    fseek(image, 10, SEEK_SET);
    fread(&inicio_pixeles, sizeof(inicio_pixeles), 1, image);
    // Buscar ancho y alto en los primeros 18 bytes
    fseek(image, 18, SEEK_SET);
    fread(&ancho, sizeof(ancho), 1, image);
    fread(&alto, sizeof(alto), 1, image);
    // Fin de busqueda
    fseek(image, inicio_pixeles, SEEK_SET);
    Pixel *pixeles = Leer_pixeles(image, ancho, alto);
    for (int i = 0; i < ancho * alto; i++)
    {
        Pixel p = pixeles[i];
        // Modificacion para guardar arreglo
        H_R[p.r]++;
        H_G[p.g]++;
        H_B[p.b]++;
        // Fin modificacion
    }
    // Funciones de Cierre
    free(pixeles);
    fclose(image);
}

void Graficar_histograma(int H_R[], int H_G[], int H_B[])
{
    int H_R_reducido[ANCHO] = {0};
    int H_G_reducido[ANCHO] = {0};
    int H_B_reducido[ANCHO] = {0};
    int Max_r = 0, Max_g = 0, Max_b = 0;
    for (int i = 0; i < 256; i++)
    {
        H_R_reducido[i / 8] += H_R[i];
        H_G_reducido[i / 8] += H_G[i];
        H_B_reducido[i / 8] += H_B[i];
    }
    printf("\n");
    for (int i = 0; i < ANCHO; i++)
    {
        if (H_R_reducido[i] > Max_r)
            Max_r = H_R_reducido[i];
        if (H_G_reducido[i] > Max_g)
            Max_g = H_G_reducido[i];
        if (H_B_reducido[i] > Max_b)
            Max_b = H_B_reducido[i];
    }
    int altura;
    for (int i = ALTURA; i > 0; i--)
    {
        for (int x = 0; x < ANCHO; x++)
        {
            altura = (H_R_reducido[x] * ALTURA) / Max_r;
            if (i <= altura)
                printf(" |");
            else
                printf(" ");
        }
        printf("\n");
    }
    printf("\n");
    for (int x = 0; x < ANCHO; x++)
        printf(" %d", x);
}

#endif
