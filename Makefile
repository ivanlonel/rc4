CC       = gcc
BIN      = rc4
OBJ      = rc4.o main.o
DEPS     = rc4.h
INCS     = -I.
CFLAGS   = $(INCS) -std=c90 -mtune=native -O2 -Wall -Wextra -pedantic
LIBS     =
RM       = rm -f

.PHONY: all all-before all-after clean clean-custom

all: all-before $(BIN) all-after

clean: clean-custom
	${RM} $(OBJ) $(BIN) *~

$(BIN): $(OBJ)
	$(CC) $^ -o $@ $(LIBS)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

