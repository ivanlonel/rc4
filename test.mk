testdir  := $(prefix)/tests

KEY      := 0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF
INPUT    := < $(testdir)/dummy.txt
OUTPUT   := > $(testdir)/encrypted.txt
ARGS     := $(KEY) $(INPUT) $(OUTPUT)

#You shouldn't need to edit below this line

COMMAND  := $(BIN) $(ARGS)

VALGRIND := valgrind -v --time-stamp=yes --track-fds=yes

CALLOPTS := --tool=callgrind --branch-sim=yes --cache-sim=yes --dump-instr=yes --collect-jumps=yes --compress-strings=no --callgrind-out-file=$(testdir)/callgrind.out.%p

MEMCOPTS := --tool=memcheck --leak-check=full --show-leak-kinds=all --track-origins=yes --expensive-definedness-checks=yes

HEAPOPTS := --tool=massif --stacks=yes --massif-out-file=$(testdir)/massif.out.%p

.PHONY: memcheck callgrind massif analyze test

test: release | $(testdir)
	$(COMMAND)

analyze: debug | $(testdir)
	$(VALGRIND) $(TOOL) $(COMMAND)

memcheck: TOOL := $(MEMCOPTS)
memcheck: analyze

callgrind: TOOL := $(CALLOPTS)
callgrind: analyze

massif: TOOL := $(HEAPOPTS)
massif: analyze

$(testdir):
	mkdir $@