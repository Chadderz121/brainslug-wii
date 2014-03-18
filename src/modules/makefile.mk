WD        := $(dir $(lastword $(MAKEFILE_LIST)))
WD_MODULE := $(WD)

SRC += $(WD)module.c
