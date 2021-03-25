###############################################################################
# Makefile
#  by Alex Chadwick
#
# A makefile script for generation of a brainslug module
###############################################################################

###############################################################################
# devkitpro settings
ifeq ($(strip $(DEVKITPPC)),)
  $(error "Please set DEVKITPPC in your environment. export DEVKITPPC=<path to>devkitPPC")
endif

include $(DEVKITPPC)/base_tools

###############################################################################
# Source files

# The source files to compile.
SRC      := main.c
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

INC_DIRS += $(LIB_INC_DIRS)

###############################################################################
# Compiler settings
LD      := $(PREFIX)ld

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


LDFLAGS += $(patsubst %,-l %,$(LIBS)) $(patsubst %,-l %,$(LIBS)) \
           $(patsubst %,-L %,$(LIB_DIRS)) $(patsubst %,-L %/lib,$(LIB_DIRS))
CFLAGS  += $(patsubst %,-I %,$(INC_DIRS)) \
           $(patsubst %,-I %/include,$(LIB_DIRS)) \
           -I $(BSLUGDIR)/include


# -O2: optimise lots
# -Wall: generate lots of warnings
# -x c: compile as C code
# -std=gnu99: use the C99 standard with GNU extensions
# -nostdinc: don't include standard headers (we don't have all the symbols)
# -ffreestanding: we don't have libc; don't expect we do
# -DGEKKO: define the symbol GEKKO (used in some libogc headers)
# -DHW_RVL: define the symbol HW_RVL (used in some libogc headers)
# -mrvl: enable wii/gamecube compilation
# -mcpu=750: enable processor specific compilation
# -meabi: enable eabi specific compilation
# -mhard-float: enable hardware floating point instructions
# -fno-common: stop common variables which the loader can't understand
# -msdata-none: do not use r2 or r13 as small data areas
# -memb: enable embedded application specific compilation
# -ffunction-sections: split up functions so linker can garbage collect
# -fdata-sections: split up data so linker can garbage collect
CFLAGS   += -O2 -Wall -x c -std=gnu99 \
			-ffreestanding \
            -DGEKKO -DHW_RVL \
            -mrvl -mcpu=750 -meabi -mhard-float -fno-common \
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
