SHELL := /bin/bash

M ?= 48
N ?= 64
K ?= 48

src_fmt ?= FP32
dst_fmt ?= FP32

op ?= gemm

PIPE_REGS ?= 3
ARRAY_HEIGHT ?= 4

REDMULE_PATH ?= $(shell echo $$REDMULE_PATH)
PULP_RISCV_GCC_TOOLCHAIN ?= $(shell echo $$PULP_RISCV_GCC_TOOLCHAIN)
GVSOC_PATH ?= $(shell echo $$GVSOC_PATH)
PULP_SDK_PATH ?= $(shell echo $$PULP_SDK_PATH)
TEST_PATH ?= $(shell echo $$TEST_PATH)

GOLDEN_PATH = $(TEST_PATH)/golden-model

configure:
	$(MAKE) -C $(GOLDEN_PATH) $(op) M=$(M) N=$(N) K=$(K) SW=$(TEST_PATH)/inc fp_fmt=$(src_fmt)

	$(REDMULE_PATH)/config_gen/config_gen.sh $(REDMULE_PATH) $(src_fmt) $(dst_fmt) $(ARRAY_HEIGHT) $(PIPE_REGS)
	$(TEST_PATH)/config_gen/config_gen.sh $(TEST_PATH) $(ARRAY_HEIGHT) $(PIPE_REGS) $(src_fmt)

run:
	export PULP_RISCV_GCC_TOOLCHAIN=$(PULP_RISCV_GCC_TOOLCHAIN)
	export GVSOC_PATH=$(GVSOC_PATH)

	source $(PULP_SDK_PATH)/configs/pulp-open.sh; $(MAKE) -C $(GVSOC_PATH) all TARGETS=pulp-open-ddr; $(MAKE) -C $(TEST_PATH) clean; $(MAKE) -C $(TEST_PATH)

	mkdir -p reports

	cd reports; $(GVSOC_PATH)/install/bin/gvsoc --target=pulp-open-ddr --binary $(TEST_PATH)/BUILD/PULP/GCC_RISCV/redmule_test/redmule_test image flash run --trace=redmule

diff:
	cd .. && \
	git diff ../../../add_dramsyslib_patches/redmule/ . | grep "new file mode " -B 1 | GREP_COLORS='ms=1;32' grep "diff " --color=always && \
	git diff ../../../add_dramsyslib_patches/redmule/ . | grep "delete file mode " -B 1 | GREP_COLORS='ms=1;31' grep "diff " --color=always && \
	git diff ../../../add_dramsyslib_patches/redmule/ . | grep "diff " -A 1 | GREP_COLORS='ms=1;34' grep -v "file mode" --color=always && \
	echo "Done"

clean:
	rm -rf reports
	rm -rf ../test/BUILD
