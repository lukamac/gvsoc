if [  -n "${ZSH_VERSION:-}" ]; then
	DIR="$(readlink -f -- "${(%):-%x}")"
	SDK_HOME=$(dirname $DIR)
else
	SDK_HOME="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"
fi

export PATH=$SDK_HOME/install/bin:$PATH
export PYTHONPATH=$SDK_HOME/install/python:$PYTHONPATH
export SYSTEMC_HOME=$SDK_HOME/third_party/systemc_install
export LD_LIBRARY_PATH=${SYSTEMC_HOME}/lib64:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$SDK_HOME/third_party/DRAMSys:$LD_LIBRARY_PATH

source $SDK_HOME/third_party/pulp-sdk/v1.0.16-pulp-riscv-gcc-centos-7/sourceme.sh
export GVSOC_PATH=$SDK_HOME
export REDMULE_PATH=$SDK_HOME/pulp/pulp/redmule
export PULP_SDK_PATH=$SDK_HOME/third_party/pulp-sdk
export TEST_PATH=$SDK_HOME/pulp/pulp/redmule/test
