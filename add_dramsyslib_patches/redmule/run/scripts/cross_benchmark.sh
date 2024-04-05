#!/bin/bash
# set -e
set -x

###############################
# Basic Settings              #
###############################

results_folder="scripts/results/cross_benchmark"
timeout_second=10

###############################
# Sweep Parameters Settings   #
###############################

list_ah=(4 8 16)
list_m=(48 96 192)
list_n=(64 128 256)
list_k=(48 96 192)

###############################
# Setup Result Folder         #
###############################

mkdir -p ${results_folder}

###############################
# Test on GVSOC Side          #
###############################

for ah in ${list_ah[@]}; do
	mkdir -p ${results_folder}/array_height_${ah}
	for m in ${list_m[@]}; do
		for n in ${list_n[@]}; do
			for k in ${list_k[@]}; do
				log_file="${results_folder}/array_height_${ah}/MNK-${m}-${n}-${k}.log"
				ARRAY_HEIGHT=${ah} M=${m} N=${n} K=${k} make configure
				ARRAY_HEIGHT=${ah} M=${m} N=${n} K=${k} timeout ${timeout_second} make run | tee ${log_file}
			done
		done
	done
done