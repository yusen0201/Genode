LEMON     = lemon-1.3
LEMON_DIR = $(REP_DIR)/contrib/$(LEMON)/lemon/
LIBS    += stdcxx libm
INC_DIR += $(REP_DIR)/contrib/$(LEMON)/
INC_DIR += $(REP_DIR)/src/lib/
SRC_CC   = 	base.cc

CC_WARN  =

vpath %.cc $(LEMON_DIR)

#SHARED_LIB = yes
