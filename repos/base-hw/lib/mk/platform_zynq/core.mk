#
# \brief  Build config for Genodes core process
# \author Johannes Schlatow
# \date   2014-12-15
#

# add include paths
INC_DIR += $(REP_DIR)/src/core/include/spec/zynq
INC_DIR += $(REP_DIR)/src/core/include/spec/cortex_a9
INC_DIR += $(REP_DIR)/src/core/include/spec/xilinx

# add C++ sources
SRC_CC += platform_services.cc
SRC_CC += spec/zynq/platform_support.cc
SRC_CC += spec/cortex_a9/pic.cc
SRC_CC += spec/arm_gic/pic.cc
SRC_CC += cpu.cc

# include less specific configuration
include $(REP_DIR)/lib/mk/arm_v7/core.inc
