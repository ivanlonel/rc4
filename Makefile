CC          := gcc
RM          := rm -rf
MV          := mv -f
MKDIR       := mkdir
SYSNAME     := uname -s
MATCH       := grep -E -i

BINNAME     := rc4
SRCNAMES    := rc4.c main.c

#prefix      := $(abspath .)
prefix      := .
exec_prefix := $(prefix)

srcdir      := $(prefix)/src
includedir  := $(prefix)/include
libdir      := $(exec_prefix)/lib
bindir      := $(exec_prefix)/bin
objdir      := $(exec_prefix)/.o
asmdir      := $(prefix)/.s
preprocdir  := $(prefix)/.i
depdir      := $(prefix)/.d

BIN         := $(addprefix $(bindir)/,$(BINNAME))
SRC         := $(if $(strip $(SRCNAMES)),$(addprefix $(srcdir)/,$(SRCNAMES)),$(wildcard $(srcdir)/*.*))
OBJ         := $(patsubst $(srcdir)/%,$(objdir)/%.o,$(basename $(SRC)))
DEP         := $(patsubst $(srcdir)/%,$(depdir)/%.d,$(basename $(SRC)))

ISCLANG     := $(shell $(CC) --version 2>&1 | $(MATCH) clang)

DIR          = $(@D)

# Create temporary dependency files and rename them in a separate step,
# so that failures during the compilation won’t leave a corrupted dependency file.
DEPFLAGS     = -MT $@ -MF $(depdir)/$*.Td -MMD -MP
POSTCOMPILE  = $(MV) $(depdir)/$*.Td $(depdir)/$*.d
PRECOMPILE   =

WFLAGS      := $(if $(ISCLANG),-Weverything -Wno-disabled-macro-expansion, \
	-Wall -Wpedantic -Wextra -Wshadow -Wconversion -Wformat=1 -Wstrict-overflow=5 -Wredundant-decls \
	-Winit-self -Wpadded -Winline -Wcast-qual -Wcast-align -Wlogical-op -Wswitch-default -Wswitch-enum \
	-Wundef -Wpointer-arith -Wfloat-equal -Wwrite-strings -Wmissing-include-dirs -Wmissing-declarations \
	-Wmissing-prototypes -Wstrict-prototypes -Wbad-function-cast -Wnested-externs -Wold-style-definition)

CPPFLAGS    := -I$(includedir) -ansi -Wp,-Wall -Wp,-pedantic
CFLAGS      := -pipe $(WFLAGS)
LDFLAGS     := -L$(libdir)

RCFLAGS     := -flto -O2 -fomit-frame-pointer -fno-common -fno-ident \
	-fno-unwind-tables -fno-asynchronous-unwind-tables -fno-stack-protector
RLDFLAGS    := -flto -s
DCFLAGS     := -g3 $(if $(ISCLANG),-fstandalone-debug,-Og)
DLDFLAGS    := $(if $(shell $(SYSNAME) 2>&1 | $(MATCH) "MINGW|WINDOWS"),,-rdynamic)

# Flags not working on MacOSX
ifeq (,$(shell $(SYSNAME) 2>&1 | $(MATCH) Darwin))
	RCFLAGS  += -ffunction-sections -fdata-sections
	RLDFLAGS += -Wl,--gc-sections -Wl,--build-id=none \
		$(if $(shell $(SYSNAME) 2>&1 | $(MATCH) "CYGWIN|MINGW|WINDOWS"),,-Wl,-z,norelro)
endif

.PHONY: all clean distclean debug release

all: release

release: CPPFLAGS += -DNDEBUG
release: CFLAGS   += $(RCFLAGS)
release: LDFLAGS  += $(RLDFLAGS)
release: $(BIN)

debug: CPPFLAGS += -DDEBUG
debug: CFLAGS   += $(DCFLAGS)
debug: LDFLAGS  += $(DLDFLAGS)
debug: $(BIN)

distclean: clean
	$(RM) $(bindir)

clean:
	$(RM) $(objdir) $(asmdir) $(preprocdir) $(depdir)

$(bindir) $(objdir) $(asmdir) $(preprocdir) $(depdir):
	$(MKDIR) $@

.SUFFIXES: # Clear the suffix list to avoid confusion with unexpected implicit rules.

# Mark files matching the patterns below as precious to make
# so they won’t be automatically deleted as intermediate files.
.PRECIOUS: $(depdir)/%.d $(preprocdir)/%.i $(asmdir)/%.s

# Enable a second expansion of prerequisites for the targets below, between read-in and target-update phases.
# The prerequisites taking advantage of this are represented in escaped variable references (two $'s).
.SECONDEXPANSION:

$(BIN): $(OBJ) | $$(DIR)
	$(CC) $^ $(LDFLAGS) -o $@ $(LDLIBS)

$(objdir)/%.o: $(srcdir)/%.c $(depdir)/%.d $$(PRECOMPILE) | $$(DIR)
	$(CC) -c $< $(DEPFLAGS) $(CPPFLAGS) $(CFLAGS) $(OUTPUT_OPTION)
	$(POSTCOMPILE)

#$(objdir)/%.o: $(asmdir)/%.s | $$(DIR)
#	$(CC) -c $< $(ASFLAGS) $(OUTPUT_OPTION)
#	$(POSTCOMPILE)
#
#$(asmdir)/%.s: $(preprocdir)/%.i | $$(DIR)
#	$(CC) -S $< $(CFLAGS) -o $@
#
#$(preprocdir)/%.i: $(srcdir)/%.c $(depdir)/%.d $$(PRECOMPILE) | $$(DIR)
#	$(CC) -E $< $(DEPFLAGS) $(CPPFLAGS) -o $@

# Create a pattern rule with an empty recipe,
# so that make won’t fail if some dependency file doesn’t exist.
# Create the dependencies directory if it still doesn't exist (order-only prerequisite).
$(depdir)/%.d: | $$(DIR) ;

# Include rules from the dependency files that exist,
# unless the current goal doesn't need them.
ifeq (,$(filter $(MAKECMDGOALS), clean distclean))
-include $(DEP) # Using '-' to avoid failing on non-existent files.
endif

-include test.mk
