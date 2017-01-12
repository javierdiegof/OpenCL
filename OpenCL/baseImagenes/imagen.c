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
descriptor_image_array_size = 0; 					// N/A si es imagen 2D
descriptor.image_row_pitch = 0; 					// Si 0, =image_width* tamaño del elemento
descriptor.image_slice_pitch = 0; 					// N/A si es imagen 2D
descriptor.num_mip_levels = 0; 						// Debe de ser 0
descriptor.num_samples = 0; 						// Debe ser 0
descriptor.buffer = NULL; 							// N/A si es imagen 2D


// Sampler en el kernel
const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;
