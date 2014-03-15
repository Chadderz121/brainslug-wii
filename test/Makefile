###############################################################################
# makefile
#  by Alex Chadwick
#
# A makefile script for generation of the brainslug regression tests
###############################################################################

###############################################################################
# helper variables
C := ,
ifeq ($(OS),Windows_NT)
  EXT := .exe
else
  EXT :=
endif

###############################################################################
# Compiler settings

LDFLAGS  += -O2
CFLAGS   += -O2 -Wall -x c -std=gnu99

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
TARGET ?= $(BIN)/regression$(EXT)

###############################################################################
# Variable init

# The names of libraries to use.
LIBS     := mxml
# The source files to compile.
SRC      :=
# Phony targets
PHONY    :=
# Include directories
INC_DIRS := 
# Library directories
LIB_DIRS := 
# Tests
TEST     :=

###############################################################################
# Rule to make everything.
PHONY += all

all : regression

###############################################################################
# Recursive rules

include makefile.mk

LDFLAGS += $(patsubst %,-l %,$(LIBS)) $(patsubst %,-l %,$(LIBS)) \
           $(patsubst %,-L %,$(LIB_DIRS)) $(patsubst %,-L %/lib,$(LIB_DIRS))
CFLAGS  += $(patsubst %,-I %,$(INC_DIRS)) \
           $(patsubst %,-I %/include,$(LIB_DIRS)) -iquote src

OBJECTS := $(patsubst %.c,$(BUILD)/%.c.o,$(filter %.c,$(SRC)))
          
ifeq ($(words $(filter clean%,$(MAKECMDGOALS))),0)
  include $(patsubst %.c,$(BUILD)/%.c.d,$(filter %.c,$(SRC)))
endif

###############################################################################
# Test suite rules

PHONY += regression

regression : $(addprefix test_, $(TEST))
	
test_% : $(TARGET)
	$Q{ $(TARGET) $* && echo "Test $* passed"; } || echo "Test $* failed ($$?)"

###############################################################################
# Special build rules

# Rule to make the image file.
$(TARGET) : $(OBJECTS) $(BIN)
	$(LOG)
	$Q$(CC) $(OBJECTS) $(LDFLAGS) -o $@ 
	
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
# Clean rule

# Rule to clean files.
PHONY += clean
clean : 
	-$Qrm -rf $(BUILD)
	-$Qrm -f $(TARGET)

###############################################################################
# Phony targets

.PHONY : $(PHONY)
