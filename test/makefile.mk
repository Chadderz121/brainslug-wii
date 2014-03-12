WD     := $(dir $(lastword $(MAKEFILE_LIST)))
WD_SRC := $(WD)

SRC  += $(WD)fsm_test.c
INCLUDE += $(WD)/../src/linker
TEST += 0 1 2
SRC  += $(WD)regression.c
