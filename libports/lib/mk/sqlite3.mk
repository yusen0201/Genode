SQLITE     = sqlite-amalgamation-3080403
SQLITE_DIR = $(REP_DIR)/contrib/$(SQLITE)/
LIBS    += libc
INC_DIR += $(SQLITE_DIR)
SRC_C    = 	sqlite3.c
CC_OPT  += -DSQLITE_THREADSAFE=0

vpath %.c $(SQLITE_DIR)

#SHARED_LIB = yes
