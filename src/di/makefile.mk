WD    := $(dir $(lastword $(MAKEFILE_LIST)))
WD_DI := $(WD)

SRC += $(WD)di.c