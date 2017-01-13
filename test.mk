testdir  := $(prefix)/test

#files inside testdir that shouldn't be deleted by 'make cleantests'
PERSIST  := dummy.txt test.txt

KEY      := 0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF
INPUT    := < $(testdir)/dummy.txt
OUTPUT   := > $(testdir)/encrypted.txt
ARGS     := $(KEY) $(INPUT) $(OUTPUT)


####### You shouldn't need to edit below this line. #######


COMMAND  := $(BIN) $(ARGS)

LINT     := splint

VALGRIND := valgrind -v --time-stamp=yes --track-fds=yes

MCHKOPTS := --leak-check=full --show-leak-kinds=all --track-origins=yes --expensive-definedness-checks=yes

CALLOPTS := --tool=callgrind --branch-sim=yes --cache-sim=yes --dump-instr=yes --collect-jumps=yes \
	--compress-strings=no --callgrind-out-file=$(testdir)/callgrind.out.%p

HEAPOPTS := --tool=massif --stacks=yes --massif-out-file=$(testdir)/massif.out.%p

PROFDATA := $(addprefix $(testdir)/, \
	$(if $(ISCLANG),default.profdata,$(subst $(exec_prefix)/,,$(OBJ:%.o=%.gcda))))

.PHONY: cleantests memcheck callgrind massif analyze lint optimize profile run

run: all
	$(COMMAND)

profile: DIR     += $(testdir)
profile: CFLAGS  += -fprofile-generate=$(testdir)
profile: LDFLAGS += -fprofile-generate=$(testdir)
profile: debug

optimize: PRECOMPILE += $(PROFDATA)
optimize: CFLAGS     += -fprofile-use=$(testdir)
optimize: LDFLAGS    += -fprofile-use=$(testdir)
optimize: release

$(PROFDATA):
ifeq (,$(ISCLANG))
	$(warning Could not find profiling information file $@)
else
	llvm-profdata merge -output=$@ $(testdir)/*.profraw
endif

analyze: debug | $(testdir)
	$(VALGRIND) $(TOOL) $(COMMAND)

memcheck: TOOL := $(MCHKOPTS)
memcheck: analyze

callgrind: TOOL := $(CALLOPTS)
callgrind: analyze

massif: TOOL := $(HEAPOPTS)
massif: analyze

lint:
	$(LINT) $(SRC) $(LINTFLAGS) $(CPPFLAGS) $(TARGET_ARCH)

cleantests:
	find $(testdir) -mindepth 1 -maxdepth 1 $(addprefix ! -name ,$(PERSIST)) -exec $(RM) {} \;

$(testdir):
	$(MKDIR) $@
