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
	//================================================================================================================
	//================================================================================================================
	//================================================================================================================
	//================================================================================================================
	
	
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
    archivo = fopen("./imagenProc.cl", "r");
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
    cl_kernel kernel = clCreateKernel(program, "imagenProc", &err);
    if(err < 0) error("Error al crear el kernel\n");
	
	// Descriptor de formato de imagen 
	cl_image_format rgba_format;
	rgba_format.image_channel_order = CL_RGBA; // Orden de canales: RGBA(en realidad es, BGRA)
	rgba_format.image_channel_data_type = CL_UNSIGNED_INT8; // Enteros de 8 bits no signados

	// Descriptor de imagen
	cl_image_desc descriptor; 
	descriptor.image_type = CL_MEM_OBJECT_IMAGE2D;		// Imagen 2D
	descriptor.image_width = ancho; 					// Ancho de imagen
	descriptor.image_height = alto; 					// Alto de imagen 
	descriptor.image_depth = 0; 						// N/A si es imagen 2D
	descriptor.image_array_size = 0; 					// N/A si es imagen 2D
	descriptor.image_row_pitch = 0; 					// Si 0, =image_width* tamaño del elemento
	descriptor.image_slice_pitch = 0; 					// N/A si es imagen 2D
	descriptor.num_mip_levels = 0; 						// Debe de ser 0
	descriptor.num_samples = 0; 						// Debe ser 0
	descriptor.buffer = NULL; 							// N/A si es imagen 2D
	
	// Crear imagen de entrada y de salida 
	cl_mem d_input_image = clCreateImage(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &rgba_format, &descriptor, image,&err);
	if(err < 0) error("Error al crear la imagen de entrada\n");
	cl_mem d_output_image = clCreateImage(context, CL_MEM_WRITE_ONLY, &rgba_format, &descriptor, NULL,&err);
	if(err < 0) error("Error al crear la imagen de salida\n");
	
	// Asignar argumentos
	err = clSetKernelArg(kernel, 0, sizeof(d_input_image), &d_input_image);
	if(err < 0) error("Error al asignar la imagen de entrada al kernel.\n");
	err = clSetKernelArg(kernel, 1, sizeof(d_output_image), &d_output_image);
	if(err < 0) error("Error al asignar la imagen de salida al kernel.\n");
	
	// Encolar kernel 
	size_t dim = 2;
	size_t global_size[] = {100,100};
	size_t local_size[] = {1,1};
	size_t offset[] = {0,0};
	
	// Ejecutar el kernel 
	err = clEnqueueNDRangeKernel(command_queue, kernel, dim, offset, global_size, local_size, 0, NULL, NULL);
	if(err < 0)error("Error al encolar.\n");
	

	// Leer la imagen desde el device al host 
	size_t origen[] = {0,0,0}, region[] = {ancho,alto,1};
	err = clEnqueueReadImage(command_queue, d_output_image, CL_TRUE, origen, region, 0, 0, image,0, NULL, NULL);
	if(err < 0 ) error("Error al encolar la imagen");
	clReleaseMemObject(d_input_image);
	clReleaseMemObject(d_output_image);
	clReleaseCommandQueue(command_queue);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseContext(context);
	
	
    
	//================================================================================================================
	//================================================================================================================
	//================================================================================================================
	//================================================================================================================
    //Termina OpenCL

    //Guardar el contenido de image en la imagen de salida
    fwrite (image, ancho*alto*4, 1, bmp_salida);

    //Cerramos el archivo de salida
    fclose (bmp_salida);

    printf("\nSUCCESS!!!\n\n");
    return 0;          
    
}