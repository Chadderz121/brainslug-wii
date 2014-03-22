WD     := $(dir $(lastword $(MAKEFILE_LIST)))
WD_SRC := $(WD)

SRC += $(WD)main.c

include $(WD_SRC)apploader/makefile.mk
include $(WD_SRC)di/makefile.mk
include $(WD_SRC)libelf/makefile.mk
include $(WD_SRC)search/makefile.mk
include $(WD_SRC)modules/makefile.mk
