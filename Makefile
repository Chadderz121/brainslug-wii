###############################################################################
# makefile
#  by Alex Chadwick
#
# A makefile script for generation of the brainslug project
###############################################################################

###############################################################################
# helper variables
C := ,

###############################################################################
# devkitpro and libogc settings
ifeq ($(strip $(DEVKITPPC)),)
  $(error "Please set DEVKITPPC in your environment. export DEVKITPPC=<path to>devkitPPC")
endif

ifeq ($(OS),Windows_NT)
  $(info Compiling from $(OS))

  PORTLIBS := $(DEVKITPRO)/portlibs/ppc
  PATH := $(DEVKITPPC)/bin:$(PORTLIBS)/bin:$(PATH)
  ifeq ($(DEVKITPRO),$(subst :, ,$(DEVKITPRO)))
    DEVKITPRO := $(patsubst /$(firstword $(subst /, ,$(DEVKITPRO)))/%,$(firstword $(subst /, ,$(DEVKITPRO))):/%,$(DEVKITPRO))
    $(info DEVKITPRO corrected to $(DEVKITPRO))
  else
    $(info DEVKITPRO is $(DEVKITPRO))
  endif
  PORTLIBS := $(DEVKITPRO)/portlibs/ppc
  ifeq ($(DEVKITPPC),$(subst :, ,$(DEVKITPPC)))
    DEVKITPPC := $(patsubst /$(firstword $(subst /, ,$(DEVKITPPC)))/%,$(firstword $(subst /, ,$(DEVKITPPC))):/%,$(DEVKITPPC))
    $(info DEVKITPPC corrected to $(DEVKITPPC))
  else
    $(info DEVKITPPC is $(DEVKITPPC))
  endif
else
  $(info Compiling from Unix)

  PORTLIBS := $(DEVKITPRO)/portlibs/ppc
  $(info DEVKITPRO is $(DEVKITPRO))
  $(info DEVKITPPC is $(DEVKITPPC))
endif

###############################################################################
# Compiler settings

# The toolchain to use.
PREFIX  = powerpc-eabi-
# Tools to use
AS      = $(PREFIX)as
LD      = $(PREFIX)g++
CC      = $(PREFIX)g++
OBJDUMP = $(PREFIX)objdump
OBJCOPY = $(PREFIX)objcopy
ELF2DOL = elf2dol

LDFLAGS  += -O2 -Wl$C--gc-sections \
            -mrvl -mcpu=750 -meabi \
            -Wl$C--section-start$C.init=0x80a00000 \
            $(patsubst %,-Wl$C-Map$C%,$(strip $(MAP)))
CFLAGS   += -O2 -Wall -x c -std=gnu99 \
            -DGEKKO -DHW_RVL -D__wii__ \
            -mrvl -mcpu=750 -meabi -mhard-float \
            -msdata=eabi -memb -ffunction-sections -fdata-sections

ifdef DEBUG
else
  CFLAGS += -DNDEBUG -flto
  LDFLAGS += -flto
endif

###############################################################################
# Parameters

# Used to suppress command echo.
Q      ?= @
LOG    ?= @echo $@
# The intermediate directory for compiled object files.
BUILD  ?= build
# The output directory for compiled results.
BIN    ?= bin
# The name of the output file to generate.
TARGET ?= $(BIN)/boot.dol
# The name of the assembler listing file to generate.
LIST   ?= $(BIN)/boot.list
# The name of the map file to generate.
MAP    ?= $(BIN)/boot.map

###############################################################################
# Variable init

# The names of libraries to use.
LIBS     := ogc mxml fat
# The source files to compile.
SRC      :=
# Phony targets
PHONY    :=
# Include directories
INC_DIRS := include
# Library directories
LIB_DIRS := $(DEVKITPPC) $(DEVKITPPC)/powerpc-eabi \
            $(DEVKITPRO)/libogc $(DEVKITPRO)/libogc/lib/wii \
            $(wildcard $(DEVKITPPC)/lib/gcc/powerpc-eabi/*) \
            $(PORTLIBS) $(PORTLIBS)/wii

###############################################################################
# Rule to make everything.
PHONY += all

all : $(TARGET) $(BIN)/boot.elf

###############################################################################
# Recursive rules

include src/makefile.mk

LDFLAGS += $(patsubst %,-l %,$(LIBS)) $(patsubst %,-l %,$(LIBS)) \
           $(patsubst %,-L %,$(LIB_DIRS)) $(patsubst %,-L %/lib,$(LIB_DIRS))
CFLAGS  += $(patsubst %,-I %,$(INC_DIRS)) \
           $(patsubst %,-I %/include,$(LIB_DIRS)) -iquote src

OBJECTS := $(patsubst %.c,$(BUILD)/%.c.o,$(filter %.c,$(SRC)))
          
ifeq ($(words $(filter clean%,$(MAKECMDGOALS))),0)
  include $(patsubst %.c,$(BUILD)/%.c.d,$(filter %.c,$(SRC)))
endif

###############################################################################
# Special build rules

# Rule to make the image file.
$(TARGET) : $(BUILD)/output.elf $(BIN)
	$(LOG)
	-$Qmkdir -p $(dir $@)
	$Q$(ELF2DOL) $(BUILD)/output.elf $(TARGET) 
	
$(BIN)/boot.elf : $(BUILD)/output.elf $(BIN)
	$(LOG)
	$Qcp $< $@
	$Q$(PREFIX)strip $@
	$Q$(PREFIX)strip -g $@

# Rule to make the elf file.
$(BUILD)/output.elf : $(OBJECTS) $(LINKER) $(BUILD)
	$(LOG)
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
$(BUILD)/%.c.d: %.c
	$(LOG)
	-$Qmkdir -p $(dir $@)
	$Q$(RM) $(wildcard $@)
	$Q{ $(CC) -MP -MM -MT $(@:.d=.o) $(CFLAGS) $< > $@ \
	&& $(RM) $@.tmp; } \
	|| { $(RM) $@.tmp && false; }

###############################################################################
# Assembly listing rules

# Rule to make assembly listing.
PHONY += list
list  : $(LIST)

# Rule to make the listing file.
%.list : $(BUILD)/output.elf $(BUILD)
	$(LOG)
	-$Qmkdir -p $(dir $@)
	$Q$(OBJDUMP) -d $(BUILD)/output.elf > $@

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
