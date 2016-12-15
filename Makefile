CC       = gcc
RM       = rm -f
BIN      = rc4
SRC      = rc4.c main.c
OBJ      = $(SRC:%.c=%.o)
DEPS     = rc4.h

RCCFLAGS = -O2
DCCFLAGS = -g3 -Og
RLDFLAGS = -s
DLDFLAGS =

CPPFLAGS = -I. -std=c90
CCFLAGS  = -Wall -Wextra -Wpedantic -pipe
LDFLAGS  = -L.

.PHONY: all clean debug release

all: release

release: CCFLAGS += $(RCCFLAGS)
release: LDFLAGS += $(RLDFLAGS)
release: $(BIN)

debug: CCFLAGS += $(DCCFLAGS)
debug: LDFLAGS += $(DLDFLAGS)
debug: $(BIN)

clean:
	${RM} $(OBJ) $(BIN) *~

$(BIN): $(OBJ)
	$(CC) $^ -o $@ $(LDFLAGS) $(LDLIBS)

%.o: %.c $(DEPS)
	$(CC) -c $< -o $@ $(CPPFLAGS) $(CCFLAGS)
