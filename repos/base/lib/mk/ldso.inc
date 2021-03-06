SHARED_LIB = yes
DIR        = $(REP_DIR)/src/lib/ldso

include $(BASE_DIR)/mk/base-libs.mk

LIBS         = $(BASE_LIBS)
SRC_CC       = main.cc test.cc exception.cc file.cc dependency.cc debug.cc \
               shared_object.cc
SRC_S        = jmp_slot.s
INC_DIR     += $(DIR)/include
LD_OPT      += -Bsymbolic-functions --version-script=$(DIR)/symbol.map

ifneq ($(filter linux, $(SPECS)),)

ENTRY_POINT = _start_initial_stack
LD_OPT     += -T$(call select_from_repositories,src/ld/context_area.nostdlib.ld) \

ifneq ($(filter x86_32, $(SPECS)),)
LD_OPT     += -T$(DIR)/linux-32.ld
endif

else
ENTRY_POINT = _start
LD_OPT     += -T$(DIR)/linker.ld
endif

vpath %.cc $(DIR)

# vi:ft=make

