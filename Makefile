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

# Sanitizing the above values
prefix      := $(if $(strip $(prefix)),$(patsubst %/,%,$(strip $(prefix))),.)
exec_prefix := $(if $(strip $(exec_prefix)),$(patsubst %/,%,$(strip $(exec_prefix))),.)

srcdir      := $(prefix)/src
includedir  := $(prefix)/include
libdir      := $(exec_prefix)/lib
bindir      := $(exec_prefix)/bin
objdir      := $(exec_prefix)/.o
asmdir      := $(prefix)/.s
preprocdir  := $(prefix)/.i
depdir      := $(prefix)/.d

BIN         := $(addprefix $(bindir)/,$(strip $(BINNAME)))
SRC         := $(if $(strip $(SRCNAMES)),$(addprefix $(srcdir)/,$(strip $(SRCNAMES))),$(wildcard $(srcdir)/*.*))
OBJ         := $(patsubst $(srcdir)/%,$(objdir)/%.o,$(basename $(SRC)))
DEP         := $(patsubst $(srcdir)/%,$(depdir)/%.d,$(basename $(SRC)))

LLVM        := $(shell $(CC) --version 2>&1 | $(MATCH) clang)

WFLAGS      := $(if $(LLVM),-Weverything -Wno-disabled-macro-expansion -Wno-long-long,-Wall -Wextra\
	-Wpedantic -Wno-long-long -Wformat=1 -Wstrict-overflow=5 -Wshadow -Wconversion -Wredundant-decls\
	-Winit-self -Wpadded -Winline -Wcast-qual -Wcast-align -Wlogical-op -Wswitch-default -Wswitch-enum\
	-Wundef -Wpointer-arith -Wfloat-equal -Wwrite-strings -Wmissing-include-dirs -Wmissing-declarations\
	-Wmissing-prototypes -Wstrict-prototypes -Wbad-function-cast -Wnested-externs -Wold-style-definition)

# If the compiler suports $(1) as a flag, return it. Else, return $(2) (return empty if no $(2) was provided).
# Doesn't check flags forwarded through -Wp, -Wa or -Wl.
flagIfAvail  = $(strip $(if $(shell echo "" | $(CC) -fsyntax-only $(1) -xc - 2>&1 | $(MATCH) error),$(2),$(1)))

# Create temporary dependency files and rename them in a separate step,
# so that failures during the compilation won’t leave a corrupted dependency file.
DEPFLAGS     = -MT $@ -MF $(depdir)/$*.Td -MMD -MP
POSTCOMPILE  = $(MV) $(depdir)/$*.Td $(depdir)/$*.d

DIR          = $(patsubst %/,%,$(dir $@))

CFLAGS       = $(if $(LLVM),-emit-llvm) $(WFLAGS) $(OPTIM_LEVEL) -pipe -fno-stack-protector
CPPFLAGS    := -I$(includedir) -ansi -Wp,-Wall -Wp,-pedantic
LDFLAGS     := -L$(libdir)

RCFLAGS     := -flto -fno-common -fno-ident -fno-unwind-tables -fno-asynchronous-unwind-tables -ffunction-sections -fdata-sections
RLDFLAGS    := -flto -s
DCFLAGS     := -g3 $(call flagIfAvail,-fstandalone-debug)
DLDFLAGS    := $(call flagIfAvail,-rdynamic)

# Flags not working on MacOSX
ifeq (,$(shell $(SYSNAME) 2>&1 | $(MATCH) Darwin))
	RLDFLAGS += -Wl,--gc-sections -Wl,--build-id=none\
		$(if $(shell $(SYSNAME) 2>&1 | $(MATCH) "CYGWIN|MINGW|WINDOWS"),\
			,\
			-Wl,-z,norelro -Wl,--hash-style=gnu\
		)
else
	RLDFLAGS += -dead_strip
endif

TARGET_ARCH := -mtune=native

.PHONY: all clean distclean debug release

all: release

release: OPTIM_LEVEL := -O3
release: CPPFLAGS += -DNDEBUG
release: CFLAGS   += $(RCFLAGS)
release: LDFLAGS  += $(RLDFLAGS)
release: $(BIN)

debug: OPTIM_LEVEL := $(call flagIfAvail,-Og,-O0)
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
.PRECIOUS: $(depdir)/%.d $(preprocdir)/%.i

# Enable a second expansion of prerequisites for the targets below, between read-in and target-update phases.
# The prerequisites taking advantage of this are represented in escaped variable references (two $'s).
.SECONDEXPANSION:



$(BIN): $(if $(LLVM),$(OBJ:%.o=%.bc),$(OBJ)) | $$(DIR)
	$(CC) $^ $(LDFLAGS) -o $@ $(TARGET_ARCH) $(LDLIBS) $(LOADLIBES)

$(objdir)/%.o $(objdir)/%.bc: $(srcdir)/%.c $(depdir)/%.d $$(PROFILING_INFO) | $$(DIR)
	$(CC) -c $< $(DEPFLAGS) $(CPPFLAGS) $(CFLAGS) $(OUTPUT_OPTION) $(TARGET_ARCH)
	$(POSTCOMPILE)

$(objdir)/%.bc: $(asmdir)/%.ll $$(PROFILING_INFO) | $$(DIR)
	$(CC) -c $< -emit-llvm $(OUTPUT_OPTION)

$(objdir)/%.o: $(asmdir)/%.s | $$(DIR)
	$(CC) -c $< $(ASFLAGS) $(OUTPUT_OPTION) $(TARGET_MACH)

$(asmdir)/%.s $(asmdir)/%.ll: $(preprocdir)/%.i $$(PROFILING_INFO) | $$(DIR)
	$(CC) -S $< $(CFLAGS) -o $@ $(TARGET_ARCH)
	$(POSTCOMPILE)

$(preprocdir)/%.i: $(srcdir)/%.c $(depdir)/%.d | $$(DIR)
	$(CC) -E $< $(DEPFLAGS) $(CPPFLAGS) -o $@ $(TARGET_ARCH)

# Create a pattern rule with an empty recipe,
# so that make won’t fail if some dependency file doesn’t exist.
# Create the dependencies directory if it still doesn't exist (order-only prerequisite).
$(depdir)/%.d: | $$(DIR) ;

# Using '-' to avoid failing on non-existent files.
# Also messing with the path as a workaround to avoid triggering the target's prerequisite.
# (Simply "-include $(DEP)" would create the $(depdir) directory even if the current goal didn't need it.)
-include $(join $(dir $(DEP)),$(addprefix ./,$(notdir $(DEP))))

-include test.mak
