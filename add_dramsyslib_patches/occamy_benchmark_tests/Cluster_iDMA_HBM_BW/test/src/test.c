#include "snrt.h"

int main()
{
	float *local_x, *remote_x;
	local_x  = 0x10000000 + (0x40000 * snrt_cluster_idx());
	remote_x = 0xc0000000;
	int cluster_idx_shuffled = (snrt_cluster_idx()/4) + (snrt_cluster_idx()%4)*6;

	snrt_cluster_hw_barrier();
	// Copy data in TCDM
	if (cluster_idx_shuffled < 18 && cluster_idx_shuffled != 18)
	{
		if (snrt_is_dm_core()) {
	        size_t size = 131072;
	        snrt_dma_start_1d(local_x, remote_x, size);
	        snrt_dma_wait_all();
	    }
	}
	    
    snrt_cluster_hw_barrier();
	return 0;
}