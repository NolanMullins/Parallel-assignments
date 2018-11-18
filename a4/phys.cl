__kernel void phys(__global float *data,
				   __local float *local_result, __global float *group_result)
{

	float sum;
	float input;
	uint global_addr, local_addr;

	global_addr = get_global_id(0);
	input = data[global_addr];

	//local_addr = get_local_id(0);
	//local_result[local_addr] = input + 1;

	printf("ID: %d | input: %.1f\n",get_group_id(0), input);

	group_result[get_group_id(0)] = input+1;

	

	//barrier(CLK_LOCAL_MEM_FENCE);

	/*if(get_local_id(0) == 0) {
      sum = 0.0f;
      for(int i=0; i<get_local_size(0); i++) {
         sum += local_result[i];
      }
      group_result[get_group_id(0)] = sum;
   }*/
}