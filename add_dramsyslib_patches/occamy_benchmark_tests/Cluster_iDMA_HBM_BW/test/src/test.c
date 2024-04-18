#include "snrt.h"

int main()
{
	float *local_x, *remote_x;
	local_x  = 0x10000000 + (0x40000 * snrt_cluster_idx());
	remote_x = 0xc0000000;
	int cluster_idx_shuffled = (snrt_cluster_idx()/4) + (snrt_cluster_idx()%4)*6;

	snrt_cluster_hw_barrier(); //Global Snitch Cluster Barrier

	if (cluster_idx_shuffled < 18 && cluster_idx_shuffled != 18)
	{
		if (snrt_is_dm_core()) {
	        size_t size = 65536; //64KiB
	        snrt_dma_start_1d(local_x, remote_x, size); //Start iDMA
	        snrt_dma_wait_all(); // Wait for iDMA Finishing
	    }
	}
	    
    snrt_cluster_hw_barrier(); //Global Snitch Cluster Barrier
	return 0;
}