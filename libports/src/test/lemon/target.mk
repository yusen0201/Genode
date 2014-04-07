TARGET   = test-lemon
LIBS     = lemon stdcxx
LEMON    = $(PRG_DIR)/../../../contrib/lemon-1.3/test/
SRC_CC   = graph_test.cc

vpath graph_test.cc $(LEMON)
