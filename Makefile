CC       = gcc
RM       = rm -f
MV       = mv -f
MATCH    = grep -i -c

BIN      = rc4
SRC      = rc4.c main.c

DEP      = $(patsubst %,%.d,$(basename $(SRC)))
OBJ      = $(DEP:%.d=%.o)

DEPFLAGS = -MT $*.o -MF $*.Td -MMD -MP
WFLAGS   = -Wall -Wpedantic -Wextra -Wshadow -Wconversion -Wformat=2 -Wstrict-overflow=5 -Wpadded -Wcast-qual -Wcast-align \
	-Wlogical-op -Wfloat-equal -Wredundant-decls -Winline -Wwrite-strings -Wmissing-include-dirs -Wmissing-declarations \
	-Wmissing-prototypes -Wstrict-prototypes -Wbad-function-cast -Wnested-externs -Wold-style-definition -Winit-self

CPPFLAGS = -I. -std=c90 -Wall -pedantic
CFLAGS   = -pipe
ASFLAGS  =
LDFLAGS  = -L.

RCFLAGS  = -O2 -flto -fomit-frame-pointer -fno-common -fno-ident -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-stack-protector
DCFLAGS  = -g3
RLDFLAGS = -s
DLDFLAGS = -rdynamic

# Flags not working on Clang
ifeq ($(shell $(CC) --version 2>&1 | $(MATCH) "clang"),0)
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

.PHONY: all clean debug release

all: release

release: CPPFLAGS += -DNDEBUG
release: CFLAGS += $(RCFLAGS)
release: LDFLAGS += $(RLDFLAGS)
release: $(BIN)

debug: CPPFLAGS += -DDEBUG
debug: CFLAGS += $(DCFLAGS)
debug: LDFLAGS += $(DLDFLAGS)
debug: $(BIN)

clean:
	$(RM) $(DEP:%.d=%.Td) $(DEP) $(DEP:%.d=%.i) $(DEP:%.d=%.s) $(OBJ) *~

$(BIN): $(OBJ)
	$(CC) $^ -o $@ $(LDFLAGS) $(LDLIBS)

# Compiling in a single step might allow better optimization.
%.o: %.c %.d
	$(CC) -c $< -o $@ $(DEPFLAGS) $(CPPFLAGS) $(WFLAGS) $(CFLAGS) $(ASFLAGS)
	$(MV) $*.Td $*.d

## Prevents make from using its implicit rules
## forcing use of the rules below instead.
#.SUFFIXES:
#
#%.o: %.s
#	$(CC) -c $< -o $@ $(ASFLAGS)
#	$(MV) $*.Td $*.d
#
#%.s: %.i
#	$(CC) -S $< -o $@ $(WFLAGS) $(CFLAGS)
#
#%.i: %.c %.d
#	$(CC) -E $< -o $@ $(DEPFLAGS) $(CPPFLAGS)


# Create a pattern rule with an empty recipe
# so that make won’t fail if the dependency file doesn’t exist.
%.d: ;

# Mark files matching the patterns below as precious to make
# so they won’t be automatically deleted as intermediate files.
.PRECIOUS: %.d

# Include rules from the dependency files that exist.
# Using - to avoid failing on non-existent files.
-include $(DEP)
