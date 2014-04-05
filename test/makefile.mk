W     := $(dir $(lastword $(MAKEFILE_LIST)))
WD_SRC := $(WD)

SRC  += $(WD)fsm_test.c
INC_DIRS += $(WD)../src/linker
TEST += 0 1 2 3 4 5 6 7 8 9 10 11
SRC  += $(WD)regression.c
SRC  += $(WD)symbol_test.c
INC_DIRS += $(WD)../src/libelf
TEST += 12 13 14 15
