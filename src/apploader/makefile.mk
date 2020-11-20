WD           := $(dir $(lastword $(MAKEFILE_LIST)))
WD_APPLOADER := $(WD)

SRC += $(WD)apploader.c
SRC += $(WD)apploader_get_ios.c