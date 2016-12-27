CC          = gcc
AS          = as
RM          = rm -rf
MV          = mv -f
MKDIR       = mkdir
MATCHCOUNT  = grep -i -c

prefix      = .
exec_prefix = $(prefix)

srcdir      = $(prefix)/src
includedir  = $(prefix)/include
libdir      = $(exec_prefix)/lib
bindir      = $(exec_prefix)/bin
objdir      = $(exec_prefix)/.o
asmdir      = $(prefix)/.s
preprocdir  = $(prefix)/.i
depdir      = $(prefix)/.d

BIN         = $(bindir)/rc4
SRC         = $(addprefix $(srcdir)/,rc4.c main.c)
#SRC        = $(wildcard $(srcdir)/*.*)
OBJ         = $(patsubst $(srcdir)/%,$(objdir)/%.o,$(basename $(SRC)))
DEP         = $(patsubst $(srcdir)/%,$(depdir)/%.d,$(basename $(SRC)))
#OBJ        = $(patsubst %,$(objdir)/%.o,$(nodir $(basename $(SRC))))
#DEP        = $(patsubst %,$(depdir)/%.d,$(nodir $(basename $(SRC))))

# Create temporary dependency files and rename them in a separate step,
# so that failures during the compilation won’t leave a corrupted dependency file.
DEPFLAGS    = -MT $@ -MF $(depdir)/$*.Td -MMD -MP
POSTCOMPILE = $(MV) $(depdir)/$*.Td $(depdir)/$*.d

WFLAGS      = -Wall -Wpedantic -Wextra -Wshadow -Wconversion -Wformat=2 -Wstrict-overflow=5 -Wpadded -Wlogical-op -Wredundant-decls \
	-Wcast-qual -Wcast-align -Wpointer-arith -Wfloat-equal -Winline -Wwrite-strings -Wmissing-include-dirs -Wmissing-declarations \
	-Wmissing-prototypes -Wstrict-prototypes -Wbad-function-cast -Wnested-externs -Wold-style-definition -Winit-self

CPPFLAGS    = -I$(includedir) $(DEPFLAGS) -ansi -Wall -pedantic
CFLAGS      = $(WFLAGS) -pipe
ASFLAGS     =
LDFLAGS     = -L$(libdir)

RCFLAGS     = -O2 -flto -fomit-frame-pointer -fno-common -fno-ident -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-stack-protector
DCFLAGS     = -g3
RLDFLAGS    = -s
DLDFLAGS    = -rdynamic

# Flags not working on Clang
ifeq ($(shell $(CC) --version 2>&1 | $(MATCHCOUNT) "clang"),0)
	RCFLAGS  += -ffunction-sections -fdata-sections
	DCFLAGS  += -Og
	RLDFLAGS += -Wl,--gc-sections -Wl,--build-id=none

	# Flags not working on Windows either
	# SystemRoot is a Windows environment variable
	ifndef SystemRoot
		ifndef SYSTEMROOT
			RLDFLAGS += -Wl,-z,norelro
		endif
	endif
else
	# Only works on Clang
	WFLAGS = -Weverything
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

# Clearing the suffix list to avoid confusion with unexpected implicit rules
.SUFFIXES:

$(BIN): $(OBJ) | $(bindir)
	$(CC) $^ -o $@ $(LDFLAGS) $(LDLIBS)

# Compiling in a single step might allow better optimization.
$(objdir)/%.o: $(srcdir)/%.c $(depdir)/%.d | $(objdir) $(depdir)
	$(CC) -c $< -o $@ $(CPPFLAGS) $(CFLAGS)
	$(POSTCOMPILE)

$(objdir)/%.o: $(asmdir)/%.s | $(objdir)
	$(AS) $< -o $@ $(ASFLAGS)
	$(POSTCOMPILE)

$(asmdir)/%.s: $(preprocdir)/%.i | $(asmdir)
	$(CC) -S $< -o $@ $(CFLAGS)

$(preprocdir)/%.i: $(srcdir)/%.c $(depdir)/%.d | $(preprocdir) $(depdir)
	$(CC) -E $< -o $@ $(CPPFLAGS)

# Create a pattern rule with an empty recipe,
# so that make won’t fail if some dependency file doesn’t exist.
$(depdir)/%.d: ;

# Mark files matching the patterns below as precious to make
# so they won’t be automatically deleted as intermediate files.
.PRECIOUS: $(depdir)/%.d $(preprocdir)/%.i $(asmdir)/%.s

$(bindir) $(objdir) $(asmdir) $(preprocdir) $(depdir):
	$(MKDIR) $@

# Include rules from the dependency files that exist.
# Using - to avoid failing on non-existent files.
-include $(DEP)

