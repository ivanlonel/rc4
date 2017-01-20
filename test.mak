testdir  := $(prefix)/test

#files inside testdir that shouldn't be deleted by 'make cleantests'
PERSIST  := dummy.txt test.txt

KEY      := 0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF
INPUT    := < $(testdir)/dummy.txt
OUTPUT   := > $(testdir)/encrypted.txt
ARGS     := $(KEY) $(INPUT) $(OUTPUT)


####### You shouldn't need to edit below this line. #######


LINT     := splint

VALGRIND := valgrind -v --time-stamp=yes --track-fds=yes

MCHKOPTS := --leak-check=full --show-leak-kinds=all --track-origins=yes --expensive-definedness-checks=yes

CALLOPTS := --tool=callgrind --branch-sim=yes --cache-sim=yes --dump-instr=yes --collect-jumps=yes\
	--compress-strings=no --callgrind-out-file=$(testdir)/$(BINNAME).%p.callgrind.out

HEAPOPTS := --tool=massif --stacks=yes --massif-out-file=$(testdir)/$(BINNAME).%p.massif.out

LINTFLAGS:= -checks

COMMAND  := $(BIN) $(ARGS)

PROFDATA := $(addprefix $(testdir)/,\
	$(if $(LLVM),\
		default.profdata,\
		$(patsubst $(prefix)%$(suffix $@),%.gcda,$@)\
	)\
)

.PHONY: cleantests memcheck callgrind massif analyze lint optimize profile run

run: all
	$(COMMAND)

profile: DIR      += $(testdir)
profile: RCFLAGS  := -fprofile-generate=$(testdir) $(DCFLAGS) $(RCFLAGS) -fno-omit-frame-pointer
#profile: ASFLAGS  += -fprofile-generate=$(testdir)
profile: RLDFLAGS := -fprofile-generate=$(testdir) $(filter-out -rdynamic -s,$(DLDFLAGS) $(RLDFLAGS))
profile: release

optimize: PROFILING_INFO += $(PROFDATA)
optimize: CFLAGS  += -fprofile-use=$(testdir)
#optimize: ASFLAGS += -fprofile-use=$(testdir)
optimize: LDFLAGS += -fprofile-use=$(testdir)
optimize: release

$(PROFDATA):
ifeq (,$(LLVM))
	$(warning Could not find profiling information file $@)
else
# It may be necessary to add llvm-profdata's path to the PATH environment variable manually. 
# Examples include '/Library/Developer/CommandLineTools/usr/bin' and '/usr/lib/llvm-3.8/bin'
	llvm-profdata merge $(testdir)/*.profraw -output=$@
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
