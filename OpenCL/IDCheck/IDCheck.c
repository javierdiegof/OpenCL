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
    if(err < 0) error("Error al obtener el nombre de la plataforma\n");

    // Obtener el nombre de la plataforma
    size_t nombre_size;
    err = clGetPlatformInfo(platform, CL_PLATFORM_NAME, 0, NULL, &nombre_size);
    if(err < 0) error("Error al obtener el tamaño de la plataforma\n");
    char* nombre[nombre_size];
    err = clGetPlatformInfo(platform, CL_PLATFORM_NAME, nombre_size, nombre, NULL);
    if(err < 0) error("Error al obtener la informacion de la plataforma");
    printf("\nPlataforma: %s\n", nombre);

    // Obtener un dispositivo
    cl_device_id device;
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 1, &device, NULL);
    if(err < 0) error("Error al obtener el nombre del dispositivo");

    // Obtener el nombre de dispositivo
    err = clGetDeviceInfo(device, CL_DEVICE_NAME, 0, NULL, &nombre_size);
    if(err < 0) error("Error al obtener el el tamanio del nombre del dispositivo");
    char * nombre_dispo[nombre_size];
    err = clGetDeviceInfo(device, CL_DEVICE_NAME, nombre_size, nombre_dispo, NULL);
    if(err < 0) error("Error al obtener la informacion del dispositivo");
    printf("Dispositivo: %s\n", nombre_dispo);

    //Crear contexto
    cl_context context;
    context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    if(err < 0) error("Error al obtener la informacion del dispositivo");


    // Leer el codigo fuente del archivo .cl
    FILE *archivo;
    archivo = fopen("./IDCheck.cl", "r");
    fseek(archivo, 0, SEEK_END);
    size_t program_size = ftell(archivo);
	rewind(archivo);

    char *buffer;
    buffer = (char *) malloc(program_size+1);
    buffer[program_size] = '\0';
    fread(buffer, 1, program_size, archivo);
    cl_program program = clCreateProgramWithSource(context, 1, (const char**)&buffer,&program_size, &err);
    if(err < 0) error("Error al crear el programa cl a partir del texto");
    free(buffer);

    // Compilar programa
    err = clBuildProgram(program,1, &device, NULL, NULL, NULL);
    if(err < 0){
	    //Hubo errores
        size_t log_size;
        err = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,0,NULL, &log_size);
        if (err < 0 ) error("Error al obtener el tamanio del log del error");
        
        char *program_log;
        program_log = (char*) malloc(log_size);
        
        err = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size, program_log, NULL);
        if (err < 0) error("Error al querer obtener la informacion del error");
        printf("\n%s\n", program_log);
        free(program_log);
        exit(1);
    }
	
	// Crear cola de comandos
	cl_command_queue command_queue;
	command_queue = clCreateCommandQueue(context, device, 0, &err);
	if(err < 0) error("Error al crear la cola de comandos\n");
	
	
	// Crear kernel
    cl_kernel kernel = clCreateKernel(program, "idcheck", &err);
    if(err < 0) error("Error al crear el kernel\n");
	
	// Declarar dimensiones, número de work-items y de work-groups
	size_t dim = 2;
	size_t global_size[] = {4,4};
	size_t local_size[] = {2,2};
	size_t offset[] = {0,0};
	
	
	// Crear un buffer de output
	float output[global_size[0]*global_size[1]];
	cl_mem d_output = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(output), NULL, &err);
	if(err < 0) error("Error al crear el buffer");
	
	// Asignar d_output como argumento 0 del kernel
	err = clSetKernelArg(kernel,0, sizeof(d_output), &d_output);
	if(err < 0) error("Error al asignar el argumento 0 del kernel\n");
	
	// Ejecutar kernel (crear workspace también)
	err = clEnqueueNDRangeKernel(command_queue, kernel, dim, offset, global_size, local_size, 0, NULL, NULL);
	if(err < 0) error("Error al crear el workspace\n");
	
	
	// Leer buffer desde el dispositivo
	err = clEnqueueReadBuffer(command_queue, d_output, CL_TRUE,0,sizeof(output), output, 0, NULL, NULL);
	if(err < 0) error("Error al leef el buffer desde el dispositivo\n");
	
	// Imprimir
	printf("Resultados: \n");
	int i;
	for(i=0; i < global_size[0]*global_size[1]; i++){
		printf("%.2f", output[i]);
		if(i%global_size[0] == global_size[0] - 1){
			printf("\n");
		}
		else{
			printf("\t");
		}
	}
	

	
	
	
	clReleaseMemObject(d_output);
	clReleaseCommandQueue(command_queue);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseContext(context);
	printf("Exito!\n");
    return 0;
}