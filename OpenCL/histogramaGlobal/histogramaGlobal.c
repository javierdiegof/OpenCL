#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
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
    archivo = fopen("./histogramaGlobal.cl", "r");
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
    cl_kernel kernel = clCreateKernel(program, "histogramag", &err);
    if(err < 0) error("Error al crear el kernel\n");
	
	// Variables del host
	int N = 100*1024*1024;
	int TAM_HISTO = 256; // Rango de datos (0 - TAM_HISTO - 1)
	
	int *datos = (int *) malloc(N*sizeof(int));
	int histo[TAM_HISTO];
	
	// Inicializar arreglos 
	srand(time(NULL));
	int i;
	for(i = 0; i < N; i++){
		datos[i] = rand() % TAM_HISTO;
	}
	for(i = 0; i < TAM_HISTO; i++){
		histo[i] = 0;
	}
	
	// Crear buffers
	cl_mem d_datos = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int)*N, datos, &err);
	if(err < 0) error("Error al crear buffer1.\n");
	cl_mem d_histo = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(histo), histo, &err);
	if(err < 0) error("Error al crear buffer2.\n");
	
	// Asignar argumentos del kernel
	err = clSetKernelArg(kernel, 0, sizeof(d_datos), &d_datos);
	if(err < 0) error("Error al asignar el kernel de datos.\n");
	err = clSetKernelArg(kernel, 1, sizeof(d_histo), &d_histo);
	if(err < 0) error("Error al asignar el kernel de histo.\n");
	err = clSetKernelArg(kernel, 2, sizeof(N), &N);
	if(err < 0) error("Error al asignar el kernel de N.\n");
	
	// Encolar kernel 
	size_t dim = 1;
	size_t global_size[] = {256*15};
	size_t local_size[] = {256};
	size_t offset[] = {0};
	
	err = clEnqueueNDRangeKernel(command_queue, kernel, dim, offset, global_size, local_size, 0, NULL, NULL);
	if(err < 0)error("Error al encolar.\n");
	
	// Leer el buffer desde el dispositivo 
	err = clEnqueueReadBuffer(command_queue, d_histo, CL_TRUE, 0, sizeof(histo), histo, 0, NULL, NULL);
	if(err  < 0) error("Error al leer el buffer desde el dispositivo \n");
	
	// Hacer lo mismo, pero en el host
	int histo_host[TAM_HISTO];
	for(i=0; i<TAM_HISTO; i++){
		histo_host[i] = 0;
	}
	for(i=0; i<TAM_HISTO; i++){
		histo_host[datos[i]]++;
	}
	
	// Mostrar resultados
	for(i = 0; i< TAM_HISTO; i++){
		assert(histo_host[i] == histo[i]);
		printf("%d - %d\n", i, histo[i]);
	}
	
	free(datos);
	clReleaseMemObject(d_datos);
	clReleaseMemObject(d_histo);
	clReleaseCommandQueue(command_queue);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseContext(context);
    printf("Exito!\n");
	return 0;
}




