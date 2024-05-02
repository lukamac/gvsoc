#!/bin/bash
set -e
set -x

###############################
# Basic Settings              #
###############################

src_folder="/scratch2/chi/SoftHier_porj/gvsoc/add_dramsyslib_patches/occamy_benchmark_tests/iDMA_RedMule_LLAMA2_Attention_Linear_Layer/results/trace_log"
results_folder="/scratch2/chi/SoftHier_porj/gvsoc/add_dramsyslib_patches/occamy_benchmark_tests/iDMA_RedMule_LLAMA2_Attention_Linear_Layer/results/wave_ir"

###############################
# Setup Result Folder         #
###############################

mkdir -p ${results_folder}

for trace_file in $(ls ${src_folder}/*.idma ); do
	filename=$(basename "$trace_file")
	extracted_name="${filename%.*}"
	log_file=${results_folder}/${extracted_name}_idma.csv
	for (( quadrant = 0; quadrant < 6; quadrant++ )); do
		for (( cluster = 0; cluster < 4; cluster++ )); do
			idma_start_config_string=$(grep "chip/soc/quadrant_${quadrant}/cluster_${cluster}/idma/fe"  ${trace_file}  | grep "dmsrc" | awk -F': ' '{ print $2 }' | awk '{ print $1 }' | tr '\n' ',')
			idma_end_string=$(grep "chip/soc/quadrant_${quadrant}/cluster_${cluster}/idma/fe/completed" ${trace_file}                 | awk -F': ' '{ print $2 }' | awk '{ print $1 }' | tr '\n' ',')
			echo "quadrant_${quadrant},cluster_${cluster},idma start,${idma_start_config_string}" >> ${log_file}
			echo "quadrant_${quadrant},cluster_${cluster},idma end  ,${idma_end_string}" >> ${log_file}
		done
	done
done

for trace_file in $(ls ${src_folder}/*.redmule ); do
	filename=$(basename "$trace_file")
	extracted_name="${filename%.*}"
	log_file=${results_folder}/${extracted_name}_redmule.csv
	for (( quadrant = 0; quadrant < 6; quadrant++ )); do
		for (( cluster = 0; cluster < 4; cluster++ )); do
			redmule_config_string=$(grep "chip/soc/quadrant_${quadrant}/cluster_${cluster}/redmule" ${trace_file} | grep "Writing register #4" 	| awk -F': ' '{ print $2 }' | awk '{ print $1 }' | tr '\n' ',')
			redmule_start_string=$(grep  "chip/soc/quadrant_${quadrant}/cluster_${cluster}/redmule" ${trace_file} | grep "REDMULE_STATUS" 		| awk -F': ' '{ print $2 }' | awk '{ print $1 }' | tr '\n' ',')
			redmule_end_string=$(grep    "chip/soc/quadrant_${quadrant}/cluster_${cluster}/redmule" ${trace_file} | grep "REDMULE_SOFT_CLEAR" 	| awk -F': ' '{ print $2 }' | awk '{ print $1 }' | tr '\n' ',')
			echo "quadrant_${quadrant},cluster_${cluster},redmule config,${redmule_config_string}" 	>> ${log_file}
			echo "quadrant_${quadrant},cluster_${cluster},redmule start ,${redmule_start_string}" 	>> ${log_file}
			echo "quadrant_${quadrant},cluster_${cluster},redmule finish,${redmule_end_string}" 	>> ${log_file}
		done
	done
done



# grep "chip/soc/quadrant_0/cluster_0/idma/fe" results/trace_log/LLAMA2_Attention_Linear_Layer_SeqLen48.idma | grep "0xd" | awk -F': ' '{ print $2 }' | awk '{ print $1 }' | tr '\n' ','