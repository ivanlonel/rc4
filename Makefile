CC          := gcc
RM          := rm -rf
MV          := mv -f
MKDIR       := mkdir
SYSNAME     := uname -s
MATCH       := grep -E -i

BINNAME     := rc4
SRCNAMES    := rc4.c main.c

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

# Create temporary dependency files and rename them in a separate step,
# so that failures during the compilation won’t leave a corrupted dependency file.
DEPFLAGS     = -MT $@ -MF $(depdir)/$*.Td -MMD -MP
UPDATEDEPS   = $(MV) $(depdir)/$*.Td $(depdir)/$*.d

WFLAGS      := $(if $(shell $(CC) --version 2>&1 | $(MATCH) clang),-Weverything -Wno-disabled-macro-expansion, \
	-Wall -Wpedantic -Wextra -Wshadow -Wconversion -Wformat=1 -Wstrict-overflow=5 \
	-Wpadded -Winline -Wredundant-decls -Wcast-qual -Wcast-align -Wfloat-equal -Wlogical-op \
	-Winit-self -Wpointer-arith -Wwrite-strings -Wmissing-include-dirs -Wmissing-declarations \
	-Wmissing-prototypes -Wstrict-prototypes -Wbad-function-cast -Wnested-externs -Wold-style-definition)

CPPFLAGS    := -I$(includedir) -ansi -Wp,-Wall -Wp,-pedantic
CFLAGS      := -pipe $(WFLAGS)
ASFLAGS     := 
LDFLAGS     := -L$(libdir)

RCFLAGS     := -flto -O2 -fomit-frame-pointer -fno-common -fno-ident \
	-fno-unwind-tables -fno-asynchronous-unwind-tables -fno-stack-protector
RLDFLAGS    := -flto -s
DCFLAGS     := -g3 $(if $(shell $(CC) --version 2>&1 | $(MATCH) clang),-fstandalone-debug,-Og)
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

.SUFFIXES: # Clearing the suffix list to avoid confusion with unexpected implicit rules

$(BIN): $(OBJ) | $(bindir)
	$(CC) $^ -o $@ $(LDFLAGS) $(LDLIBS)

# Compiling in a single step often allows better compiler optimization.
$(objdir)/%.o: $(srcdir)/%.c $(depdir)/%.d | $(objdir)
	$(CC) -c $< -o $@ $(DEPFLAGS) $(CPPFLAGS) $(CFLAGS)
	$(UPDATEDEPS)

$(objdir)/%.o: $(asmdir)/%.s | $(objdir)
	$(CC) -c $< -o $@ $(ASFLAGS)
	$(UPDATEDEPS)

$(asmdir)/%.s: $(preprocdir)/%.i | $(asmdir)
	$(CC) -S $< -o $@ $(CFLAGS)

$(preprocdir)/%.i: $(srcdir)/%.c $(depdir)/%.d | $(preprocdir)
	$(CC) -E $< -o $@ $(DEPFLAGS) $(CPPFLAGS)

# Create a pattern rule with an empty recipe,
# so that make won’t fail if some dependency file doesn’t exist.
# Create the dependencies directory if it still doesn't exist (order-only prerequisite).
$(depdir)/%.d: | $(depdir) ;

# Mark files matching the patterns below as precious to make
# so they won’t be automatically deleted as intermediate files.
.PRECIOUS: $(depdir)/%.d $(preprocdir)/%.i $(asmdir)/%.s

$(bindir) $(objdir) $(asmdir) $(preprocdir) $(depdir):
	$(MKDIR) $@

# Include rules from the dependency files that exist,
# unless the current goal doesn't need them.
ifeq (,$(filter $(MAKECMDGOALS), clean distclean))
-include $(DEP) # Using '-' to avoid failing on non-existent files.
endif
