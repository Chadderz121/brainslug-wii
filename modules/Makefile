###############################################################################
# Makefile
#  by Alex Chadwick
#
# A makefile script for generation of all brainslug modules
###############################################################################

MODULES += console-gx
MODULES += console-sd
MODULES += gecko
MODULES += https
MODULES += libfat
MODULES += libfat-sd
MODULES += libsd
MODULES += template

###############################################################################
# Parameters

# Used to suppress command echo.
Q      ?= @
LOG    ?= @echo $@

###############################################################################
# Rule to make everything.
PHONY += all
all: $(addsuffix _module,$(MODULES))

%_module:
	$Q$(MAKE) -C $*
	
###############################################################################
# Install rule

# Rule to install bslug.
PHONY += install
install : $(addsuffix _module_install,$(MODULES))
	
%_module_install: | $(BSLUGDIR)/modules
	$(LOG)
	$(addprefix $Qrm -rf ,$(wildcard $(BSLUGDIR)/modules/$*))
	$Qcp -r $* $(BSLUGDIR)/modules
	
$(BSLUGDIR)/modules:
	$Qmkdir $(BSLUGDIR)/modules

# Rule to install bslug.
PHONY += uninstall
uninstall : 
	$(LOG)
	$(addprefix $Qrm -rf ,$(wildcard $(BSLUGDIR)/modules))

###############################################################################
# Clean rule

# Rule to clean files.
PHONY += clean
clean: $(addsuffix _module_clean,$(MODULES))
	
%_module_clean:
	$Q$(MAKE) -C $* clean

###############################################################################
# Phony targets

.PHONY : $(PHONY)