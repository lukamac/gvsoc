#!/bin/bash
set -e
set -x

###############################
# Basic Settings              #
###############################

results_folder="/scratch2/chi/SoftHier_porj/gvsoc/add_dramsyslib_patches/occamy_benchmark_tests/iDMA_RedMule_LLAMA2_Attention_Linear_Layer/results/trace_log"
app_path="/scratch2/chi/SoftHier_porj/gvsoc/third_party/occamy/target/sim/sw/device/apps/blas/test/src/test.c"
sw_path="/scratch2/chi/SoftHier_porj/gvsoc/third_party/occamy/target/sim"
execution_path="/scratch2/chi/SoftHier_porj/gvsoc"

###############################
# Sweep Parameters Settings   #
###############################

sequence_length_list=(48 96 192)
attention_dimension_list=(128)

###############################
# Setup Result Folder         #
###############################

mkdir -p ${results_folder}

###############################
# Design Parameter Testing    #
###############################

for at_size in ${attention_dimension_list[@]}; do
	for seq_len in ${sequence_length_list[@]}; do
		#modify soruce code
		sed -i "17s/.*/#define SEQ_LEN ${seq_len}/" ${app_path}
		sed -i "18s/.*/#define ATTENTION_DIMENSION ${at_size}/" ${app_path}
		cd ${sw_path}
		make DEBUG=ON sw
		cd ${execution_path}

		#get idma trace
		./install/bin/gvsoc --target=occamy \
		--binary third_party/occamy/target/sim/sw/host/apps/offload/build/offload-test.elf \
		--debug-binary third_party/occamy/target/sim/sw/device/apps/blas/test/build/test.elf run \
		--trace-level=5 \
		--trace=idma > ${results_folder}/LLAMA2_Attention_Linear_Layer_Attension${at_size}_SeqLen${seq_len}.idma

		#get redmule trace
		./install/bin/gvsoc --target=occamy \
		--binary third_party/occamy/target/sim/sw/host/apps/offload/build/offload-test.elf \
		--debug-binary third_party/occamy/target/sim/sw/device/apps/blas/test/build/test.elf run \
		--trace-level=5 \
		--trace=redmule > ${results_folder}/LLAMA2_Attention_Linear_Layer_Attension${at_size}_SeqLen${seq_len}.redmule
	done
done