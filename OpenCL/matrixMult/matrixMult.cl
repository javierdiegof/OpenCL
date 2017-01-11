__kernel void matrixMult(__global float* A,__global float* B,__global float* C, int N){
	
	int i;
	size_t idxC = get_global_id(0)* get_global_size(1) + get_global_id(1);
	float temp = 0;
	
	for(i=0;i<N;i++){
		size_t idxA = get_global_id(0)*N + i;
		size_t idxB = get_global_id(1) + get_global_size(1)*i;
		temp += A[idxA] * B[idxB];
	}
	C[idxC] = temp;
}