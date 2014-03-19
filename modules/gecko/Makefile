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
INC_DIRS := ../../include
# Library directories
LIB_DIRS :=
# The names of libraries to use.
LIBS     :=
# The output directory for compiled results.
BIN      := bin
# The name of the output file to generate.
TARGET 	 := $(BIN)/$(notdir $(CURDIR)).mod

###############################################################################
# Hopefully you shouldn't need to change anything after this line
###############################################################################

###############################################################################
# helper variables
C := ,

###############################################################################
# devkitpro and libogc settings
ifeq ($(strip $(DEVKITPPC)),)
  $(error "Please set DEVKITPPC in your environment. export DEVKITPPC=<path to>devkitPPC")
endif

PATH := $(DEVKITPPC)/bin:$(PATH)

###############################################################################
# Compiler settings

# The toolchain to use.
PREFIX  = powerpc-eabi-
# Tools to use
AS      = $(PREFIX)as
LD      = $(PREFIX)ld
CC      = $(PREFIX)g++

LDFLAGS  += --relocatable -s \
            -T ../../include/bslug/bslug.ld \
            $(patsubst %,-Map %,$(strip $(MAP)))
CFLAGS   += -O2 -Wall -x c -std=gnu99 \
            -DGEKKO -DHW_RVL -D__wii__ \
            -mrvl -mcpu=750 -meabi -mhard-float \
            -msdata=none -memb -ffunction-sections -fdata-sections

ifdef DEBUG
else
  CFLAGS += -DNDEBUG
endif

###############################################################################
# Parameters

# Used to suppress command echo.
Q      ?= @
LOG    ?= @echo $@
# The intermediate directory for compiled object files.
BUILD  ?= build
# The name of the assembler listing file to generate.
LIST   ?= $(TARGET:.mod=.list)
# The name of the map file to generate.
MAP    ?= $(TARGET:.mod=.list)

###############################################################################
# Variable init

# Phony targets
PHONY    :=

###############################################################################
# Rule to make everything.
PHONY += all

all : $(TARGET)

###############################################################################
# Derived rules

LDFLAGS += $(patsubst %,-l %,$(LIBS)) $(patsubst %,-l %,$(LIBS)) \
           $(patsubst %,-L %,$(LIB_DIRS)) $(patsubst %,-L %/lib,$(LIB_DIRS))
CFLAGS  += $(patsubst %,-I %,$(INC_DIRS)) \
           $(patsubst %,-I %/include,$(LIB_DIRS)) -iquote src

OBJECTS := $(patsubst %.c,$(BUILD)/%.c.o,$(filter %.c,$(SRC)))

###############################################################################
# Special build rules

# Rule to make the module file.
$(TARGET) : $(OBJECTS) $(BIN)
	$(LOG)
	-$Qmkdir -p $(dir $@)
	$Q$(LD) $(OBJECTS) $(LDFLAGS) -o $@ 
	
# Rule to make intermediate directory
$(BUILD) : 
	-$Qmkdir $@

# Rule to make output directory
$(BIN) : 
	-$Qmkdir $@

###############################################################################
# Standard build rules

$(BUILD)/%.c.o: %.c
	$(LOG)
	-$Qmkdir -p $(dir $@)
	$Q$(CC) -c $(CFLAGS) $< -o $@

###############################################################################
# Assembly listing rules

# Rule to make assembly listing.
PHONY += list
list  : $(LIST)

# Rule to make the listing file.
%.list : $(TARGET)
	$(LOG)
	-$Qmkdir -p $(dir $@)
	$Q$(OBJDUMP) -d $< > $@

###############################################################################
# Clean rule

# Rule to clean files.
PHONY += clean
clean : 
	-$Qrm -rf $(BUILD)
	-$Qrm -f $(TARGET)
	-$Qrm -f $(LIST)
	-$Qrm -f $(MAP)

###############################################################################
# Phony targets

.PHONY : $(PHONY)
