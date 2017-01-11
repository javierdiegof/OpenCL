__kernel void idcheck(__global float* output){

	size_t global_id_0 = get_global_id(0);
	size_t global_id_1 = get_global_id(1);
	
	size_t group_id_0 = get_group_id(0);
	size_t group_id_1 = get_group_id(1);

	size_t local_id_0 = get_local_id(0);
	size_t local_id_1 = get_local_id(1);
	
	size_t global_size_0 = get_global_size(0);
	size_t global_size_1 = get_global_size(1);
	
	size_t local_size_0 = get_size_id(0);
	size_t local_size_1 = get_size_id(1);
	
	size_t global_offset_0 = get_global_offset(0);
	size_t global_offset_1 = get_global_offset(1);
	
	size_t num_groups_0 = get_num_groups(0);
	size_t num_groups_1 = get_num_groups(1);
	
	// Determinar el indice del arreglo
	size_t index0 = global_id_0 - global_offset_0;
	size_t index1 = global_id_1 - global_ offset_1;
	// Se hace el mapeo del valor
	size_t index = index1*global_size_0 + index0;
}