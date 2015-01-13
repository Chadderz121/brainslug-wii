###############################################################################
# Makefile
#  by Alex Chadwick
#
# A makefile script for generation of a brainslug module
###############################################################################
# Hopefully you shouldn't need to change anything in this file.
# Alter makefile.mk to change common settings.

###############################################################################
# devkitpro settings
ifeq ($(strip $(DEVKITPRO)),)
  $(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>devkitPro")
endif
ifeq ($(strip $(DEVKITPPC)),)
  $(error "Please set DEVKITPPC in your environment. export DEVKITPPC=<path to>devkitPPC")
endif

BSLUGDIR := $(DEVKITPRO)/bslug
GCC_VER := $(shell $(DEVKITPPC)/bin/powerpc-eabi-gcc -dumpversion)

PATH := $(DEVKITPPC)/bin:$(PATH)
LIB_INC_DIRS := $(DEVKITPPC)/lib/gcc/powerpc-eabi/$(GCC_VER)/include \
                $(DEVKITPPC)/lib/gcc/powerpc-eabi/$(GCC_VER)/include-fixed \
                $(BSLUGDIR)/include

include makefile.mk

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
LIST   ?= $(TARGET:.mod=.list)
# The name of the map file to generate.
MAP    ?= $(TARGET:.mod=.map)

INC_DIRS += $(LIB_INC_DIRS)

###############################################################################
# Compiler settings

# The toolchain to use.
PREFIX  ?= powerpc-eabi-
# Tools to use
AS      := $(PREFIX)as
LD      := $(PREFIX)ld
CC      := $(PREFIX)g++
OBJDUMP := $(PREFIX)objdump

# --relocatable: make sure ld doesn't remove relocations bslug will need
# -s: strip local symbols to speed linking
# --gc-sections: remove unneeded symbols
# -T: use the linker script specified (to force certain bslug sections together)
# -Map: generate a map file
LDFLAGS  += --relocatable -s --gc-sections -u bslug_load -u bslug_meta \
            -T $(BSLUGDIR)/bslug.ld \
            $(patsubst %,-Map %,$(strip $(MAP)))
LD1FLAGS += --relocatable -s \
            -T $(BSLUGDIR)/bslug_elf.ld

# -O2: optimise lots
# -Wall: generate lots of warnings
# -x c: compile as C code
# -std=gnu99: use the C99 standard with GNU extensions
# -nostdinc: don't include standard headers (we don't have all the symbols)
# -ffreestanding: we don't have libc; don't expect we do
# -DGEKKO: define the symbol GEKKO (used in some libogc headers)
# -DHW_RVL: define the symbol HW_RVL (used in some libogc headers)
# -D__wii__: define the symbol __wii__ (used in some libogc headers)
# -mrvl: enable wii/gamecube compilation
# -mcpu=750: enable processor specific compilation
# -meabi: enable eabi specific compilation
# -mhard-float: enable hardware floating point instructions
# -fshort-wchar: use 16 bit whcar_t type in keeping with Wii executables
# -fno-common: stop common variables which the loader can't understand
# -msdata-none: do not use r2 or r13 as small data areas
# -memb: enable embedded application specific compilation
# -ffunction-sections: split up functions so linker can garbage collect
# -fdata-sections: split up data so linker can garbage collect
CFLAGS   += -O2 -Wall -x c -std=gnu99 \
			-nostdinc -ffreestanding \
            -DGEKKO -DHW_RVL -D__wii__ \
            -mrvl -mcpu=750 -meabi -mhard-float -fshort-wchar -fno-common \
            -msdata=none -memb -ffunction-sections -fdata-sections

ifdef DEBUG
else
  CFLAGS += -DNDEBUG
endif

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
$(TARGET) : $(BUILD)/output.elf | $(BIN)
	$(LOG)
	$Q$(LD) $(BUILD)/output.elf $(LDFLAGS) -o $@ 

# Rule to make the module file.
$(BUILD)/output.elf : $(OBJECTS) | $(BIN) $(BUILD)
	$(LOG)
	$Q$(LD) $(OBJECTS) $(LD1FLAGS) -o $@ 
	
# Rule to make intermediate directory
$(BUILD) : 
	$Qmkdir $@

# Rule to make output directory
$(BIN) : 
	$Qmkdir $@

###############################################################################
# Standard build rules

$(BUILD)/%.c.o: %.c | $(BUILD)
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
	$Qrm -rf $(wildcard $(BUILD) $(BIN))

###############################################################################
# Phony targets

.PHONY : $(PHONY)
