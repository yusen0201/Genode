TARGET   = test-lemon_lgf
LIBS     = lemon stdcxx libc_fs libc_log
LEMON    = $(PRG_DIR)/../../../contrib/lemon-1.3/demo/
SRC_CC   = lgf_demo.cc

vpath lgf_demo.cc $(LEMON)
