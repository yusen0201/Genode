TARGET   = test-soplex
LIBS     = soplex libc_log stdcxx
SOPLEX   = $(PRG_DIR)/../../../contrib/soplex-2.0.0/src/
SRC_CC   = example.cpp

vpath example.cpp $(SOPLEX)
