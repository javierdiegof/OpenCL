__kernel void imagenProc(read_only image2d_t input, write_only image2d_t output){
	// Sampler en el kernel
	const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;
	int index_x = get_global_id(0);
	int index_y = get_global_id(1);
	int tam_x = get_image_width(input);
	int tam_y = get_image_height(input);
	int pos_x;
	int2 coordn;
	while(index_y < tam_y){
		while(index_x < tam_x){
			if(index_x < tam_x/2){
				pos_x = (tam_x / 2) + (tam_x/2 - index_x);
				coordn = (int2)(pos_x, index_y);
			}
			else{
				coordn = (int2)(index_x, index_y);
			}
			int2 coordo = (int2)(index_x, index_y);
			uint4 pixel = read_imageui(input, sampler, coordn);
			uint4 new_pixel = pixel;
			write_imageui(output, coordo, new_pixel);
			index_x += get_global_size(0);
		}
		index_x = get_global_id(0);
		index_y += get_global_size(1);
	}
}