#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>

int error(const char string[]){
    perror(string);
    exit(1);
}

int main (int argc, char* argv[]){

    //Abrir archivo de entrada
    FILE *bmp_entrada;
    if ((bmp_entrada = fopen(argv[1],"rb")) == NULL) 
        error("Error al abrir el archivo de entrada");

    //Comprobar cabecera identificadora del BMP
    char isBMP[3]="";
    fread(isBMP, 2, 1, bmp_entrada);
    if (strcmp(isBMP,"BM")){
        fclose(bmp_entrada);
        error("La imagen debe ser un bitmap");
    }

    //Comprobar bits por pixel
    unsigned int bitsxpixel;
    fseek (bmp_entrada, 28, SEEK_SET);
    fread (&bitsxpixel, 4, 1, bmp_entrada);
    if (bitsxpixel != 32){
        fclose(bmp_entrada);
        error("La imagen debe ser de 32 bits\n");
    }

    //Leer tamaño de la cabecera de datos
    unsigned int tamano_cabecera;
    fseek(bmp_entrada, 10, SEEK_SET);
    fread(&tamano_cabecera, 4, 1, bmp_entrada);
    
    //Leer ancho y alto de la imagen
    unsigned int ancho, alto;
    fseek (bmp_entrada, 18, SEEK_SET);
    fread (&ancho, 4, 1, bmp_entrada);
    fread (&alto, 4, 1, bmp_entrada);
    
    //Abrir imagen de salida, escritura y binario
    FILE *bmp_salida;
    if ((bmp_salida = fopen(argv[2],"wb")) == NULL){
        fclose(bmp_entrada);
        error("Error al abrir el archivo de salida");
    }
    
    //Trasladar cabecera del archivo de entrada al de salida
    fseek(bmp_entrada, 0, SEEK_SET);
    int i;
    for (i=0; i<tamano_cabecera; i++)
        fputc(fgetc(bmp_entrada), bmp_salida);

    //Guardar pixeles de la imagen de entrada en arreglo image[alto*ancho*4]
    unsigned char* image;
    image = (unsigned char*)malloc(ancho*alto*4);
    fread (image, ancho*alto*4, 1, bmp_entrada);
    fclose (bmp_entrada);

    //Inicia OpenCL

    

    //Termina OpenCL

    //Guardar el contenido de image en la imagen de salida
        fwrite (image, ancho*alto*4, 1, bmp_salida);

    //Cerramos el archivo de salida
    fclose (bmp_salida);

    printf("\nSUCCESS!!!\n\n");
    return 0;          
    
}