GLUCOSE     = glucose-3.0
GLUCOSE_DIR = $(REP_DIR)/contrib/$(GLUCOSE)
LIBS    += stdcxx zlib libc libm
INC_DIR += $(REP_DIR)/contrib/$(GLUCOSE)
SRC_CC   = 	Solver.cc

CC_OPT += -D__STDC_LIMIT_MACROS -D__STDC_FORMAT_MACROS

CC_WARN  =

vpath %.cc $(GLUCOSE_DIR)/core
vpath %.cc $(GLUCOSE_DIR)/utils

#SHARED_LIB = yes
