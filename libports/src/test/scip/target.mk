TARGET = test-scip
LIBS   = libc scip stdcxx libc_log config_args
SCIP   = scip-3.1.0
SCIP_DIR = $(REP_DIR)/contrib/$(SCIP)/
EX_DIR   = $(SCIP_DIR)/examples/Queens/src
INC_DIR  = $(EX_DIR)
SRC_CC = queens.cpp queens_main.cpp

vpath %.cpp $(EX_DIR)
