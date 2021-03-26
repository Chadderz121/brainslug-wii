###############################################################################
# Makefile
#  by Alex Chadwick
#
# A makefile script for generation of a brainslug module
###############################################################################

###############################################################################
# Source files

# The source files to compile.
SRC      := gecko.c
# Include directories
INC_DIRS := 
# Library directories
LIB_DIRS :=
# The names of libraries to use.
LIBS     :=
# The output directory for compiled results.
BIN      := bin
# The name of the output file to generate.
TARGET 	 := $(BIN)/$(notdir $(CURDIR)).mod
# C compiler flags
CFLAGS   :=

###############################################################################
# Parameters

# A comma to make writing commands easier
C := ,
# Used to suppress command echo.
Q      ?= @
LOG    ?= @echo $@
# The intermediate directory for compiled object files.
BUILD  ?= build
# The name of the assembler listing file to generate.
LIST   ?= $(TARGET).list
# The name of the map file to generate.
MAP    ?= $(TARGET).map

ifeq ($(strip $(BSLUGDIR)),)
  $(error "Please set BSLUGDIR in your environment. export BSLUGDIR=<path to>bslug")
endif

include $(BSLUGDIR)/bslug.mk
