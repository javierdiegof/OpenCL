__kernel void imagenProc(read_only image2d_t input, write_only image2d_t output){
	// Sampler en el kernel
	const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;
}