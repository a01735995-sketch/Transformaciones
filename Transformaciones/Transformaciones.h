#ifndef TRANSFORMACIONES_H
#define TRANSFORMACIONES_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>

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

static const float Kernel[n][m] = {
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

/* La cabecera se conserva; el signo de la altura solo se normaliza en memoria.
 * outputImage puede ser NULL para validar sin crear una salida. */
int Leer_cabecera(FILE *image, FILE *outputImage, int32_t *ancho, int32_t *alto)
{
    unsigned char cabecera[54];
    uint32_t inicio_pixeles, dib, compresion;
    int32_t altura_archivo;
    uint16_t planos, bits;
    if (!image || fseek(image, 0, SEEK_SET) != 0 ||
        fread(cabecera, 1, sizeof(cabecera), image) != sizeof(cabecera))
        return 0;
    if (cabecera[0] != 'B' || cabecera[1] != 'M') return 0;
    /* BMP usa little endian; esta version admite hosts little endian. */
    const uint16_t endian = 1;
    if (*(const unsigned char *)&endian != 1) return 0;
    if (fseek(image, 10, SEEK_SET) != 0 ||
        fread(&inicio_pixeles, 4, 1, image) != 1 ||
        fread(&dib, 4, 1, image) != 1 ||
        fread(ancho, 4, 1, image) != 1 ||
        fread(&altura_archivo, 4, 1, image) != 1 ||
        fread(&planos, 2, 1, image) != 1 ||
        fread(&bits, 2, 1, image) != 1 ||
        fread(&compresion, 4, 1, image) != 1) return 0;
    if (dib < 40 || (uint64_t)inicio_pixeles < 14u + (uint64_t)dib ||
        *ancho <= 0 || altura_archivo == 0 || altura_archivo == INT32_MIN ||
        planos != 1 || bits != 24 || compresion != 0) return 0;
    *alto = altura_archivo < 0 ? -altura_archivo : altura_archivo;
    uint64_t cantidad = (uint64_t)*ancho * (uint64_t)*alto;
    uint64_t fila = ((uint64_t)*ancho * 3u + 3u) & ~(uint64_t)3u;
    uint64_t esperado = (uint64_t)inicio_pixeles + fila * (uint64_t)*alto;
    if (cantidad > INT_MAX || cantidad > SIZE_MAX / sizeof(Pixel) ||
        esperado > LONG_MAX || fseek(image, 0, SEEK_END) != 0) return 0;
    long longitud = ftell(image);
    if (longitud < 0 || (uint64_t)longitud < esperado ||
        fseek(image, 0, SEEK_SET) != 0) return 0;
    for (uint32_t j = 0; j < inicio_pixeles; j++) {
        int dato = fgetc(image);
        if (dato == EOF || (outputImage && fputc(dato, outputImage) == EOF))
            return 0;
    }
    return 1;
}

Pixel *Leer_pixeles(FILE *image, int32_t ancho, int32_t alto)
{
    if (ancho <= 0 || alto <= 0 ||
        (uint64_t)ancho * (uint64_t)alto > SIZE_MAX / sizeof(Pixel)) return NULL;
    Pixel *pixeles = malloc((size_t)ancho * (size_t)alto * sizeof(Pixel));
    if (!pixeles) return NULL;
    int relleno = (int)((4u - ((uint64_t)ancho * 3u) % 4u) % 4u);
    for (int fila = 0; fila < alto; fila++) {
        if (fread(&pixeles[(size_t)fila * (size_t)ancho], sizeof(Pixel),
                  (size_t)ancho, image) != (size_t)ancho ||
            fseek(image, relleno, SEEK_CUR) != 0) {
            free(pixeles);
            return NULL;
        }
    }
    return pixeles;
}

void Escribir_relleno(FILE *outputImage, int32_t ancho)
{
    unsigned char relleno[3] = {0};
    fwrite(relleno, 1, (size_t)((4u - ((uint64_t)ancho * 3u) % 4u) % 4u), outputImage);
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
    Pixel_gris *pixeles_gris = malloc((size_t)ancho * (size_t)alto * sizeof(Pixel_gris));
    if (!pixeles_gris) return NULL;
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

int escala_grises(const char *salida, const char *entrada)
{
    // Declarar arreglos de imagenes a leer y a guardar
    FILE *image = fopen(entrada, "rb");
    if (!image) return 0;
    FILE *outputImage = fopen(salida, "wb");
    if (!outputImage) { fclose(image); return 0; }
    // Fin declaracion
    int32_t ancho, alto;
    if (!Leer_cabecera(image, outputImage, &ancho, &alto)) {
        fclose(image); fclose(outputImage); return 0;
    }
    Pixel *pixeles = Leer_pixeles(image, ancho, alto);
    if (!pixeles) { fclose(image); fclose(outputImage); return 0; }
    Pixel_gris *pixeles_gris = Convertir_grises(pixeles, ancho, alto);
    if (!pixeles_gris) { free(pixeles); fclose(image); fclose(outputImage); return 0; }
    for (int i = 0; i < ancho * alto; i++)
    {
        fwrite(&pixeles_gris[i], sizeof(Pixel_gris), 1, outputImage);
        if ((i + 1) % ancho == 0)
            Escribir_relleno(outputImage, ancho);
    }
    Copiar_final(image, outputImage);
    // Funciones de Cierre
    int correcto = !ferror(image) && !ferror(outputImage);
    if (fclose(image) != 0) correcto = 0;
    if (fclose(outputImage) != 0) correcto = 0;
    free(pixeles);
    free(pixeles_gris);
    return correcto;
}

int inv_img(const char *salida, const char *entrada)
{
    // Declarar arreglos de imagenes a leer y a guardar
    FILE *image = fopen(entrada, "rb");
    if (!image) return 0;
    FILE *outputImage = fopen(salida, "wb");
    if (!outputImage) { fclose(image); return 0; }
    // Fin declaracion
    int32_t ancho, alto;
    if (!Leer_cabecera(image, outputImage, &ancho, &alto)) {
        fclose(image); fclose(outputImage); return 0;
    }
    Pixel *pixeles = Leer_pixeles(image, ancho, alto);
    if (!pixeles) { fclose(image); fclose(outputImage); return 0; }
    Pixel_gris *pixeles_gris = Convertir_grises(pixeles, ancho, alto);
    if (!pixeles_gris) { free(pixeles); fclose(image); fclose(outputImage); return 0; }
    // Recorrer el arreglo al reves gira la imagen 180 grados
    for (int x = ancho * alto - 1; x >= 0; x--)
    {
        fwrite(&pixeles_gris[x], sizeof(Pixel_gris), 1, outputImage);
        if (x % ancho == 0)
            Escribir_relleno(outputImage, ancho);
    }
    // Fin del giro de 180 grados
    Copiar_final(image, outputImage);
    // Funciones de Cierre
    int correcto = !ferror(image) && !ferror(outputImage);
    if (fclose(image) != 0) correcto = 0;
    if (fclose(outputImage) != 0) correcto = 0;
    free(pixeles);
    free(pixeles_gris);
    return correcto;
}

int Parcial_x(const char *salida, const char *entrada) {
    // Declarar arreglos de imagenes a leer y a guardar
    FILE *image = fopen(entrada, "rb");
    if (!image) return 0;
    FILE *outputImage = fopen(salida, "wb");
    if (!outputImage) { fclose(image); return 0; }
    // Fin declaracion
    int32_t ancho, alto;
    if (!Leer_cabecera(image, outputImage, &ancho, &alto)) {
        fclose(image); fclose(outputImage); return 0;
    }
    Pixel *pixeles = Leer_pixeles(image, ancho, alto);
    if (!pixeles) { fclose(image); fclose(outputImage); return 0; }
    // Impresion derivada parcial
    /* Diferencias por parejas dentro de cada fila; un pixel final impar se conserva. */
    for (int fila = 0; fila < alto; fila++) {
        for (int columna = 0; columna + 1 < ancho; columna += 2) {
            int x = fila * ancho + columna;
            pixeles[x].b = (unsigned char)(pixeles[x + 1].b - pixeles[x].b);
            pixeles[x].r = (unsigned char)(pixeles[x + 1].r - pixeles[x].r);
            pixeles[x].g = (unsigned char)(pixeles[x + 1].g - pixeles[x].g);
            pixeles[x + 1].b = pixeles[x + 1].r = pixeles[x + 1].g = 0;
        }
        fwrite(&pixeles[fila * ancho], sizeof(Pixel), (size_t)ancho, outputImage);
        Escribir_relleno(outputImage, ancho);
    }
    Copiar_final(image, outputImage);
    // Funciones de Cierre
    int correcto = !ferror(image) && !ferror(outputImage);
    if (fclose(image) != 0) correcto = 0;
    if (fclose(outputImage) != 0) correcto = 0;
    free(pixeles);
    return correcto;
}

int inv_img_color(const char *salida, const char *entrada)
{
    // Declarar arreglos de imagenes a leer y a guardar
    FILE *image = fopen(entrada, "rb");
    if (!image) return 0;
    FILE *outputImage = fopen(salida, "wb");
    if (!outputImage) { fclose(image); return 0; }
    // Fin declaracion
    int32_t ancho, alto;
    if (!Leer_cabecera(image, outputImage, &ancho, &alto)) {
        fclose(image); fclose(outputImage); return 0;
    }
    Pixel *pixeles = Leer_pixeles(image, ancho, alto);
    if (!pixeles) { fclose(image); fclose(outputImage); return 0; }
    // Recorrer el arreglo al reves gira la imagen 180 grados
    for (int x = ancho * alto - 1; x >= 0; x--)
    {
        fwrite(&pixeles[x], sizeof(Pixel), 1, outputImage);
        if (x % ancho == 0)
            Escribir_relleno(outputImage, ancho);
    }
    // Fin del giro de 180 grados
    Copiar_final(image, outputImage);
    // Funciones de Cierre
    int correcto = !ferror(image) && !ferror(outputImage);
    if (fclose(image) != 0) correcto = 0;
    if (fclose(outputImage) != 0) correcto = 0;
    free(pixeles);
    return correcto;
}

int inv_img_grey_horizontal(const char *salida, const char *entrada)
{
    // Declarar arreglos de imagenes a leer y a guardar
    FILE *image = fopen(entrada, "rb");
    if (!image) return 0;
    FILE *outputImage = fopen(salida, "wb");
    if (!outputImage) { fclose(image); return 0; }
    // Fin declaracion
    int32_t ancho, alto;
    if (!Leer_cabecera(image, outputImage, &ancho, &alto)) {
        fclose(image); fclose(outputImage); return 0;
    }
    Pixel *pixeles = Leer_pixeles(image, ancho, alto);
    if (!pixeles) { fclose(image); fclose(outputImage); return 0; }
    Pixel_gris *pixeles_gris = Convertir_grises(pixeles, ancho, alto);
    if (!pixeles_gris) { free(pixeles); fclose(image); fclose(outputImage); return 0; }
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
    int correcto = !ferror(image) && !ferror(outputImage);
    if (fclose(image) != 0) correcto = 0;
    if (fclose(outputImage) != 0) correcto = 0;
    free(pixeles);
    free(pixeles_gris);
    return correcto;
}

int inv_img_color_horizontal(const char *salida, const char *entrada)
{
    // Declarar arreglos de imagenes a leer y a guardar
    FILE *image = fopen(entrada, "rb");
    if (!image) return 0;
    FILE *outputImage = fopen(salida, "wb");
    if (!outputImage) { fclose(image); return 0; }
    // Fin declaracion
    int32_t ancho, alto;
    if (!Leer_cabecera(image, outputImage, &ancho, &alto)) {
        fclose(image); fclose(outputImage); return 0;
    }
    Pixel *pixeles = Leer_pixeles(image, ancho, alto);
    if (!pixeles) { fclose(image); fclose(outputImage); return 0; }
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
    int correcto = !ferror(image) && !ferror(outputImage);
    if (fclose(image) != 0) correcto = 0;
    if (fclose(outputImage) != 0) correcto = 0;
    free(pixeles);
    return correcto;
}

int desenfoque(const char *entrada, const char *salida)
{
    // Declarar arreglos de imagenes a leer y a guardar
    FILE *image = fopen(entrada, "rb");
    if (!image) return 0;
    FILE *outputImage = fopen(salida, "wb");
    if (!outputImage) { fclose(image); return 0; }
    // Fin declaracion
    int32_t ancho, alto;
    if (!Leer_cabecera(image, outputImage, &ancho, &alto)) {
        fclose(image); fclose(outputImage); return 0;
    }
    Pixel *pixeles = Leer_pixeles(image, ancho, alto);
    if (!pixeles) { fclose(image); fclose(outputImage); return 0; }
    Pixel *pixeles_blur = malloc((size_t)ancho * (size_t)alto * sizeof(Pixel));
    if (!pixeles_blur) { free(pixeles); fclose(image); fclose(outputImage); return 0; }
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
    int correcto = !ferror(image) && !ferror(outputImage);
    if (fclose(image) != 0) correcto = 0;
    if (fclose(outputImage) != 0) correcto = 0;
    free(pixeles);
    free(pixeles_blur);
    return correcto;
}

int desenfoque_gris(const char *entrada, const char *salida)
{
    // Declarar arreglos de imagenes a leer y a guardar
    FILE *image = fopen(entrada, "rb");
    if (!image) return 0;
    FILE *outputImage = fopen(salida, "wb");
    if (!outputImage) { fclose(image); return 0; }
    // Fin declaracion
    int32_t ancho, alto;
    if (!Leer_cabecera(image, outputImage, &ancho, &alto)) {
        fclose(image); fclose(outputImage); return 0;
    }
    Pixel *pixeles = Leer_pixeles(image, ancho, alto);
    if (!pixeles) { fclose(image); fclose(outputImage); return 0; }
    Pixel_gris *pixeles_gris = Convertir_grises(pixeles, ancho, alto);
    if (!pixeles_gris) { free(pixeles); fclose(image); fclose(outputImage); return 0; }
    Pixel_gris *pixeles_gris_blur = malloc((size_t)ancho * (size_t)alto * sizeof(Pixel_gris));
    if (!pixeles_gris_blur) { free(pixeles); free(pixeles_gris); fclose(image); fclose(outputImage); return 0; }
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
    int correcto = !ferror(image) && !ferror(outputImage);
    if (fclose(image) != 0) correcto = 0;
    if (fclose(outputImage) != 0) correcto = 0;
    free(pixeles);
    free(pixeles_gris);
    free(pixeles_gris_blur);
    return correcto;
}

int Calcular_histograma(const char *entrada, int H_R[], int H_G[], int H_B[])
{
    FILE *image = fopen(entrada, "rb");
    if (!image) return 0;
    int32_t ancho, alto;
    if (!Leer_cabecera(image, NULL, &ancho, &alto)) { fclose(image); return 0; }
    Pixel *pixeles = Leer_pixeles(image, ancho, alto);
    if (!pixeles) { fclose(image); return 0; }
    for (int i = 0; i < ancho * alto; i++) {
        Pixel p = pixeles[i];
        H_R[p.r]++; H_G[p.g]++; H_B[p.b]++;
    }
    free(pixeles);
    return fclose(image) == 0;
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
            altura = Max_r ? (int)((int64_t)H_R_reducido[x] * ALTURA / Max_r) : 0;
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
