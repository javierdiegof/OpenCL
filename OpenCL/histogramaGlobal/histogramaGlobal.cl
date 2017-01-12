__kernel void histogramag(__global int* datos, __global int* histo, int N){
	
	int index = get_global_id(0);
	while(index<N){
		atomic_inc(&histo[datos[index]]);
		index += get_global_size(0);
	}

}




