#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>

int error(char string[]){
    perror(string);
    exit(1);
}

int main(){
    // Obtener una plataforma
    cl_platform_id platform;
    cl_int err;
    err = clGetPlatformIDs(1, &platform, NULL);

    // Obtener el nombre de la plataforma
    size_t nombre_size;
    err = clGetPlatformInfo(platform, CL_PLATFORM_NAME, 0, NULL, &nombre_size);
    char* nombre[nombre_size];
    err = clGetPlatformInfo(platform, CL_PLATFORM_NAME, nombre_size, nombre, NULL);
    printf("\nPlataforma: %s\n", nombre);

    // Obtener un dispositivo
    cl_device_id device;
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 1, &device, NULL);

    // Obtener el nombre de dispositivo
    err = clGetDeviceInfo(device, CL_DEVICE_NAME, 0, NULL, &nombre_size);
    char * nombre_dispo[nombre_size];
    err = clGetDeviceInfo(device, CL_DEVICE_NAME, nombre_size, nombre_dispo, NULL);
    printf("Dispositivo: %s\n", nombre_dispo);


    return 0;

}




