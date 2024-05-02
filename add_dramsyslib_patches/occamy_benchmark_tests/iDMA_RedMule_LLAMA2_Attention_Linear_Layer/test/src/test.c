#include "snrt.h"
#include "hal_redmule.h"
#include "inc/config.h"

uint32_t L = 16;

float a = 2;

float x[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

float y[16] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1,  1,  1,  1,  1,  1};

float z[16];

float w[16];

#define SEQ_LEN 192
#define ATTENTION_DIMENSION 128
#define REDMULE_KSIZE ((ATTENTION_DIMENSION*3)/8)

void RedMule_Test()
{
	snrt_cluster_hw_barrier();

	if (snrt_cluster_core_idx() == 0)
	{
		redmule_cfg(M_SIZE, N_SIZE, K_SIZE, gemm_ops);
		redmule_x_add_set(x);
		redmule_w_add_set(w);
		redmule_z_add_set(z);
		redmule_y_add_set(y);
		hwpe_trigger_job();
		hwpe_get_status();
		hwpe_soft_clear();
		//second run
		redmule_x_add_set(x);
		redmule_y_add_set(y);
		hwpe_trigger_job();
		hwpe_get_status();
		hwpe_soft_clear();
	}

	snrt_cluster_hw_barrier();
}


void iDMA_RedMule_Test(float *local_x, float *remote_e, float *remote_w, uint32_t M, uint32_t N, uint32_t K)
{
    if (snrt_is_dm_core()) {
        size_t size;

        size = M*N;
        snrt_dma_start_1d(local_x, remote_e, size);
        snrt_dma_wait_all();

        size = N*K;
        snrt_dma_start_1d(local_x, remote_w, size);
        snrt_dma_wait_all();
    }

	if (snrt_cluster_core_idx() == 0)
	{
		redmule_cfg(M, N, K, gemm_ops);
		redmule_x_add_set(x);
		redmule_w_add_set(w);
		redmule_z_add_set(z);
		redmule_y_add_set(y);
		hwpe_trigger_job();
		hwpe_get_status();
		hwpe_soft_clear();
	}
}


void iDMA_RedMule_Configured_Test(float *local_x, float *remote_e, float *remote_w, uint32_t M, uint32_t N, uint32_t K)
{
    if (snrt_is_dm_core()) {
        size_t size;

        size = M*N;
        snrt_dma_start_1d(local_x, remote_e, size);
        snrt_dma_wait_all();

        size = N*K;
        snrt_dma_start_1d(local_x, remote_w, size);
        snrt_dma_wait_all();
    }

	if (snrt_cluster_core_idx() == 0)
	{
		hwpe_trigger_job();
		hwpe_get_status();
		hwpe_soft_clear();
	}
}


void LLAMA2_Attension_Linear()
{
	//Decription:
	// 	For each head attension: 
	//		Embeding Matrix 		= [Sequence Length 	x 5120 					]
	// 		Weight Matrix (Q,K,V)   = [5120				x Attention Dimension 	]
	// 		Linear Layer Output		= [Sequence Length 	x Attention Dimension 	]
	//  When using RedMule:
	//		Sequence Length 		= (48, 	96,  192)
	//		Attention Dimension 	= (128, 256, 512)
	// 		block size 				= [Sequence Length 	x 256 	x REDMULE_KSIZE]
	// 		Number of Total Iters 	= [5120/256] x 3 = 60
	//  Matrix Address Mapping:
	// 		Embeding Matrix 		= 0xc0000000;
	//      Weight Matrix 			= 0xd0000000;

	float *local_x, *remote_e, *remote_w;
	local_x  = 0x10000000 + (0x40000 * snrt_cluster_idx());
	remote_e = 0xc0000000;
	remote_w = 0xd0000000 + (ATTENTION_DIMENSION * 5120 * snrt_cluster_idx());

	//RedMule Initialization
	snrt_cluster_hw_barrier();
	if (snrt_cluster_core_idx() == 0)
	{
		redmule_cfg(SEQ_LEN, 256, REDMULE_KSIZE, gemm_ops);
		redmule_x_add_set(x);
		redmule_w_add_set(w);
		redmule_z_add_set(z);
		redmule_y_add_set(y);
	}

	//First round: load initial blcoks
	snrt_cluster_hw_barrier();
	if (snrt_is_dm_core()) {
        size_t size;

        //load 1st Embeding Tile
        size = SEQ_LEN * 256;
        snrt_dma_start_1d(local_x, remote_e, size);
        snrt_dma_wait_all();

        // load 1st Weight Tile
        size = 256 * REDMULE_KSIZE;
        snrt_dma_start_1d(local_x, remote_w, size);
        snrt_dma_wait_all();
    }
    snrt_cluster_hw_barrier();

    //Routin: RedMule + iDMA
    for (int i = 1; i < 59; ++i)
    {
    	remote_e += SEQ_LEN * 256;
    	if (i%20 == 0)
    	{
    		remote_e = 0xc0000000;
    	}
    	remote_w += 256 * REDMULE_KSIZE;
    	iDMA_RedMule_Configured_Test(local_x, remote_e, remote_w, SEQ_LEN, 256, REDMULE_KSIZE);
    	snrt_cluster_hw_barrier();
    }

    //Last Computation Round: RedMule Only
    if (snrt_cluster_core_idx() == 0)
	{
		redmule_x_add_set(x);
		redmule_y_add_set(y);
		hwpe_trigger_job();
		hwpe_get_status();
		hwpe_soft_clear();
	}
	snrt_cluster_hw_barrier();

}

void iDMA_Quadrant_Transfer()
{
	float *local_x, *remote_x;
	uint32_t quadrant_idx = snrt_cluster_idx()/4;
	uint32_t cluster_in_qudrant_idx = snrt_cluster_idx()%4;
	uint32_t next_cluster_in_qudrant_idx = (cluster_in_qudrant_idx + 1)%4;
	uint32_t next_cluster_idx = quadrant_idx * 4 + next_cluster_in_qudrant_idx;

	local_x  = 0x10000000 + (0x40000 * snrt_cluster_idx());
	remote_x = 0x10000000 + (0x40000 * next_cluster_idx);

	snrt_cluster_hw_barrier();

	if ((snrt_cluster_idx() < 3) && snrt_is_dm_core())
	{
		size_t size = 65536;
        snrt_dma_start_1d(local_x, remote_x, size);
        snrt_dma_wait_all();
	}

	snrt_cluster_hw_barrier();
}

void iDMA_Mutual_Cluster_Transfer()
{
	float *local_x, *remote_x;
	uint32_t next_cluster_idx = (snrt_cluster_idx() == 0)? 1:0;
	local_x  = 0x10000000 + (0x40000 * snrt_cluster_idx());
	remote_x = 0x10000000 + (0x40000 * next_cluster_idx);

	snrt_cluster_hw_barrier();

	if ((snrt_cluster_idx() < 2) && snrt_is_dm_core())
	{
		size_t size = 131072;
        snrt_dma_start_1d(local_x, remote_x, size);
        snrt_dma_wait_all();
	}

	snrt_cluster_hw_barrier();
}

int main()
{
	LLAMA2_Attension_Linear();
	return 0;
}