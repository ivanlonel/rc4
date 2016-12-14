CC       = gcc
BIN      = rc4
SRC      = rc4.c main.c
OBJ      = $(SRC:%.c=%.o)
DEPS     = rc4.h
LIBS     = -L.
INCS     = -I.
RELFLAGS = -mtune=native -O2
DBGFLAGS = -g3 -Og
CFLAGS   = $(INCS) -std=c90 -Wall -Wextra -pedantic $(RELFLAGS)
RM       = rm -f

.PHONY: all clean debug

all: $(BIN) 

clean:
	${RM} $(OBJ) $(BIN) *~

debug: CFLAGS := $(patsubst $(RELFLAGS),$(DBGFLAGS),$(CFLAGS))
debug: $(BIN)

$(BIN): $(OBJ)
	$(CC) $^ -o $@ $(LIBS)

%.o: %.c $(DEPS)
	$(CC) -c $< -o $@ $(CFLAGS)

