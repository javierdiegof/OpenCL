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
    archivo = fopen("./matrixMult.cl", "r");
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
    cl_kernel kernel = clCreateKernel(program, "matrixMult", &err);
    if(err < 0) error("Error al crear el kernel\n");

	// Definir matrices A, B y C
	int M = 3,
		N = 5, 
		P = 4;
		
	float A[M][N], B[N][P], C[M][P];

	// Inicializar matrices
	int i,j;
	for(i=0;i<M;i++){
		for(j=0; j<N; j++){
			A[i][j] = 1;
		}
	}
	for(i=0;i < N; i++){
		for(j=0; j< P; j++){
			B[i][j] = 1;
		}
	}
	
	// Crear buffers de A, B y C
	cl_mem d_A = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(A), A, &err);
	if(err < 0) error("Error\n");
	
	cl_mem d_B = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(B), B, &err);
	if(err < 0) error("Error\n");
	
	cl_mem d_C = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(C), NULL, &err);
	if(err < 0) error("\n");
	
	
	// Asignar A, B y C como argumentos del kernel
	err = clSetKernelArg(kernel, 0, sizeof(d_A), &d_A);
	if(err < 0) error("Error al asignar la matriz A al kernel");
	
	err = clSetKernelArg(kernel, 1, sizeof(d_B), &d_B);
	if(err < 0) error("Error al asignar la matriz B al kernel");
	
	err = clSetKernelArg(kernel, 2, sizeof(d_C), &d_C);
	if(err < 0) error("Error al asignar la matriz C al kernel");
	
	err = clSetKernelArg(kernel, 3, sizeof(N), &N);
	if(err < 0) error("Error al asignar la variable N al argumento ");
	
	
	// Declarar dimensiones, número de work-items y de work-groups
	size_t dim = 2;
	size_t global_size[] = {M,P};
	size_t local_size[] = {M,P};
	size_t offset[] = {0,0};

	
	
	// Ejecutar kernel (crear workspace también)
	err = clEnqueueNDRangeKernel(command_queue, kernel, dim, offset, global_size, local_size, 0, NULL, NULL);
	if(err < 0) printf("%d", err);
	
	
	// Leer buffer desde el dispositivo
	err = clEnqueueReadBuffer(command_queue, d_C, CL_TRUE,0,sizeof(C), 	C, 0, NULL, NULL);
	if(err < 0) error("Error al leef el buffer desde el dispositivo\n");
	
	printf("Llega hasta aqui\n.");
	// Imprimir resultados
	printf("\n\n Matriz A: \n");
	for(i=0; i < M; i++){
		for(j=0; j < N; j++){
			printf("%f\t",A[i][j]);
		}
		printf("\n");
	}
	
	printf("\n\n Matriz B: \n");
	for(i=0; i < N; i++){
		for(j=0; j < P; j++){
			printf("%f\t",B[i][j]);
		}
		printf("\n");
	}
	
	printf("\n\n Matriz C: \n");
	for(i=0; i < M; i++){
		for(j=0; j < P; j++){
			printf("%f\t",C[i][j]);
		}
		printf("\n");
	}
	
	
	clReleaseMemObject(d_A);
	clReleaseMemObject(d_B);
	clReleaseMemObject(d_C);
	clReleaseCommandQueue(command_queue);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseContext(context);
	printf("Exito!\n");
    return 0;
}