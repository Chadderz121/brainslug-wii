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
# devkitpro settings
ifeq ($(strip $(DEVKITPPC)),)
  $(error "Please set DEVKITPPC in your environment. export DEVKITPPC=<path to>devkitPPC")
endif

BSLUGDIR ?= $(CURDIR)/bslug

include $(DEVKITPPC)/base_tools

PORTLIBS := $(DEVKITPRO)/portlibs/ppc

ELF2DOL ?= elf2dol
LD := $(CC)

# -O2: optimise lots
# -Wl$C--gc-sections: remove unneeded symbols
# -mrvl: enable wii/gamecube compilation
# -mcpu=750: enable processor specific compilation
# -meabi: enable eabi specific compilation
# -Wl$C--section-start$C.init=0x80a00000:
#    start the executable after 0x80a00000 so we don't have to move in order to
#    load a dol file from a disk.
# -Wl$C-Map$C: generate a map file
LDFLAGS  += -O2 -Wl$C--gc-sections \
            -mrvl -mcpu=750 -meabi \
            -Wl$C--section-start$C.init=0x80a00000 \
            $(patsubst %,-Wl$C-Map$C%,$(strip $(MAP)))
# -O2: optimise lots
# -Wall: generate lots of warnings
# -x c: compile as C code
# -std=gnu99: use the C99 standard with GNU extensions
# -DGEKKO: define the symbol GEKKO (used in some libogc headers)
# -DHW_RVL: define the symbol HW_RVL (used in some libogc headers)
# -D__wii__: define the symbol __wii__ (used in some libogc headers)
# -mrvl: enable wii/gamecube compilation
# -mcpu=750: enable processor specific compilation
# -meabi: enable eabi specific compilation
# -mhard-float: enable hardware floating point instructions
# -msdata=eabi: use r2 and r13 as small data areas
# -memb: enable embedded application specific compilation
# -ffunction-sections: split up functions so linker can garbage collect
# -fdata-sections: split up data so linker can garbage collect
CFLAGS   += -O2 -Wall -x c -std=gnu99 \
            -DGEKKO -DHW_RVL -D__wii__ \
            -mrvl -mcpu=750 -meabi -mhard-float \
            -msdata=eabi -memb -ffunction-sections -fdata-sections

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
# The output directory for compiled results.
BIN    ?= bin
# The output directory for releases.
RELEASE?= release
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
INC_DIRS := . $(DEVKITPRO)/libogc/include $(PORTLIBS)/include
# Library directories
LIB_DIRS :=  $(DEVKITPRO)/libogc/lib/wii $(PORTLIBS)/lib

###############################################################################
# Rule to make everything.
PHONY += all

all : $(TARGET) $(BIN)/boot.elf

###############################################################################
# Install rule

# Rule to install bslug.
PHONY += install
install : bslug_include modules symbols bslug.ld  bslug_elf.ld
	$(LOG)
	$(addprefix $Qrm -rf ,$(wildcard $(BSLUGDIR)))
	$Qmkdir $(BSLUGDIR)
	$Qcp -r bslug_include $(BSLUGDIR)/include
	$Q$(MAKE) -C modules install BSLUGDIR=$(BSLUGDIR)
	$Qcp -r symbols $(BSLUGDIR)
	$Qcp -r bslug.ld $(BSLUGDIR)
	$Qcp -r bslug_elf.ld $(BSLUGDIR)

# Rule to install bslug.
PHONY += uninstall
uninstall : 
	$(LOG)
	$(addprefix $Qrm -rf ,$(wildcard $(BSLUGDIR)))
	
###############################################################################
# Release rules

PHONY += release
release: $(TARGET) meta.xml icon.png
	$(LOG)
	$(addprefix $Qrm -rf ,$(wildcard $(RELEASE)))
	$Qmkdir $(RELEASE)
	$Qmkdir $(RELEASE)/apps
	$Qmkdir $(RELEASE)/apps/brainslug
	$Qcp -r $(TARGET) $(RELEASE)/apps/brainslug
	$Qcp -r meta.xml $(RELEASE)/apps/brainslug
	$Qcp -r icon.png $(RELEASE)/apps/brainslug
	$Qmkdir $(RELEASE)/bslug
	$Qcp -r symbols $(RELEASE)/bslug
	$Qmkdir $(RELEASE)/bslug/modules
	$Qcp -r USAGE $(RELEASE)/readme.txt

###############################################################################
# Recursive rules

include src/makefile.mk

LDFLAGS += $(patsubst %,-l %,$(LIBS)) $(patsubst %,-l %,$(LIBS)) \
           $(patsubst %,-L %,$(LIB_DIRS)) $(patsubst %,-L %/lib,$(LIB_DIRS))
CFLAGS  += $(patsubst %,-I %,$(INC_DIRS)) \
           -iquote src

OBJECTS := $(patsubst %.c,$(BUILD)/%.c.o,$(filter %.c,$(SRC)))
          
ifeq ($(words $(filter clean%,$(MAKECMDGOALS))),0)
ifeq ($(words $(filter install%,$(MAKECMDGOALS))),0)
ifeq ($(words $(filter uninstall%,$(MAKECMDGOALS))),0)
  -include $(patsubst %.c,$(BUILD)/%.c.d,$(filter %.c,$(SRC)))
endif
endif
endif

###############################################################################
# Special build rules

# Rule to make the image file.
$(TARGET) : $(BUILD)/output.elf | $(BIN)
	$(LOG)
	-$Qmkdir -p $(dir $@)
	$Q$(ELF2DOL) $(BUILD)/output.elf $(TARGET) 
	
$(BIN)/boot.elf : $(BUILD)/output.elf | $(BIN)
	$(LOG)
	$Qcp $< $@
	$Q$(PREFIX)strip $@
	$Q$(PREFIX)strip -g $@

# Rule to make the elf file.
$(BUILD)/output.elf : $(OBJECTS) $(LINKER) | $(BIN) $(BUILD)
	$(LOG)
	$Q$(LD) $(OBJECTS) $(LDFLAGS) -o $@ 

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
$(BUILD)/%.c.d: %.c | $(BUILD)
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
	$Qrm -rf $(wildcard $(BUILD) $(BIN) $(RELEASE))

###############################################################################
# Phony targets

.PHONY : $(PHONY)
