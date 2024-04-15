source sourceme.sh
CXX=g++-11.2.0 CC=gcc-11.2.0 make checkout
CXX=g++-11.2.0 CC=gcc-11.2.0 make redmule_perparation
CXX=g++-11.2.0 CC=gcc-11.2.0 make dramsys_preparation
pip3 install -r core/requirements.txt --user
pip3 install -r gapy/requirements.txt --user
pip3 install dataclasses --user
CXX=g++-11.2.0 CC=gcc-11.2.0 CMAKE=cmake-3.18.1 make TARGETS=pulp-open-ddr all
CXX=g++-11.2.0 CC=gcc-11.2.0 CMAKE=cmake-3.18.1 make TARGETS=occamy all
./install/bin/gvsoc --target=pulp-open-ddr --binary add_dramsyslib_patches/dma_dram_test.bin image flash run --trace=ddr
./install/bin/gvsoc --target=occamy --binary examples/occamy/offload-multi_cluster.elf --debug-binary examples/occamy/multi_cluster.elf run
make occamy_pdk_preparation
./install/bin/gvsoc --target=occamy --binary third_party/occamy/target/sim/sw/host/apps/offload/build/offload-test.elf --debug-binary third_party/occamy/target/sim/sw/device/apps/blas/test/build/test.elf run
