#
# \brief  Build config for Genodes core process
# \author Johannes Schlatow
# \date   2016-01-15
#

# add include paths
INC_DIR += $(REP_DIR)/src/core/include/spec/parallella

# include less specific configuration
include $(REP_DIR)/lib/mk/spec/zynq/core.inc
