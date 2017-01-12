// En cada grupo se va reserva esa memoria local
__kernel void histogramal(__global int* datos, __global int* histo, __local int* cache, int N){
	
	int index = get_global_id(0);
	// Inicializamos la caché en 0
	cache[get_local_id(0)] = 0; 
	barrier(CLK_LOCAL_MEM_FENCE);
	
	// Calculamos histograma parcial
	while(index<N){
		atomic_inc(&cache[datos[index]]);
		index += get_global_size(0);
	}
	barrier(CLK_LOCAL_MEM_FENCE);
	
	// Acumulamos resultados parciales
	atomic_add(&histo[get_local_id(0)], cache[get_local_id(0)]);
}




