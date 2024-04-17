#execution to log files
./install/bin/gvsoc --target=occamy --binary third_party/occamy/target/sim/sw/host/apps/offload/build/offload-test.elf --debug-binary third_party/occamy/target/sim/sw/device/apps/blas/test/build/test.elf run --trace=idma/axi --trace-level=6 > log_end.txt
./install/bin/gvsoc --target=occamy --binary third_party/occamy/target/sim/sw/host/apps/offload/build/offload-test.elf --debug-binary third_party/occamy/target/sim/sw/device/apps/blas/test/build/test.elf run --trace=chip/soc/hbm_mst_ports_ --trace-level=6 > log_start.txt
./install/bin/gvsoc --target=occamy --binary third_party/occamy/target/sim/sw/host/apps/offload/build/offload-test.elf --debug-binary third_party/occamy/target/sim/sw/device/apps/blas/test/build/test.elf run --trace=wide_axi --trace-level=6 > log_cluster_start.txt

#extract results
echo ""
echo "************************************************************"
echo "**              Snitch iDMA Transfer Results              **"
echo "************************************************************"
end_string=$(grep "axi_response" log_end.txt | tail -n 1)
end_cycle=$(echo "$end_string" | grep -oP '(?<=: )\d+')
start_string=$(grep "Received IO req" log_start.txt | head -n 1)
start_cycle=$(echo "$start_string" | grep -oP '^[^:]*: \K\d+')
echo "      		start_cycle	|	end_cycle"
echo "Total:		${start_cycle}		|	${end_cycle}"
echo "------------------------------------------------------------"


for (( cluster = 0; cluster < 4; cluster++ )); do
	for (( quadrant = 0; quadrant < 6; quadrant++ )); do
		cluster_end_string=$(grep "chip/soc/quadrant_${quadrant}/cluster_${cluster}" log_end.txt | grep "axi_response" | tail -n 1)
		cluster_end_cycle=$(echo "$cluster_end_string" | grep -oP '(?<=: )\d+')
		cluster_start_string=$(grep "chip/soc/quadrant_${quadrant}/cluster_${cluster}/wide_axi" log_cluster_start.txt | grep "0xc0000000" | head -n 1)
		cluster_start_cycle=$(echo "$cluster_start_string" | grep -oP '^[^:]*: \K\d+')
		echo "Q${quadrant}_C${cluster}:		${cluster_start_cycle}		|	${cluster_end_cycle}"
	done
done

#remove log file
rm -rf log_end.txt log_start.txt log_cluster_start.txt