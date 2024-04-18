set -x

for (( i = 1; i <= 24; i++ )); do
	sed -i "28s/.*/#define NUM_ENABLED_CORES ${i}/"  /scratch2/chi/SoftHier_porj/gvsoc/third_party/occamy/target/sim/sw/device/apps/blas/test/src/test.c
	cd /scratch2/chi/SoftHier_porj/gvsoc/third_party/occamy/target/sim
	make DEBUG=ON sw
	cd /scratch2/chi/SoftHier_porj/gvsoc
	echo "" >> result.txt 
	echo "###################################### NUM_ENABLED_CORES = ${i} ###############################################" >> result.txt 
	echo "" >> result.txt 
	source add_dramsyslib_patches/occamy_benchmark_tests/Cluster_iDMA_HBM_BW/scripts/extract_dma_results.sh >> result.txt 
done