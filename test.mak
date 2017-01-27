testdir  := $(prefix)/test

#files inside testdir that shouldn't be deleted by 'make cleantests'
PERSIST  := dummy.txt test.txt

KEY      := 00102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f202122232425262728292a2b2c2d2e2f303132333435363738393a3b3c3d3e3f404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f606162636465666768696a6b6c6d6e6f707172737475767778797a7b7c7d7e7f808182838485868788898a8b8c8d8e8f909192939495969798999a9b9c9d9e9fA0A1A2A3A4A5A6A7A8A9AaAbAcAdAeAfB0B1B2B3B4B5B6B7B8B9BaBbBcBdBeBfC0C1C2C3C4C5C6C7C8C9CaCbCcCdCeCfD0D1D2D3D4D5D6D7D8D9DaDbDcDdDeDfE0E1E2E3E4E5E6E7E8E9EaEbEcEdEeEfF0F1F2F3F4F5F6F7F8F9FaFbFcFdFeFf
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
profile: RLDFLAGS := -fprofile-generate=$(testdir) $(DLDFLAGS) $(filter -flto,$(RLDFLAGS))
profile: release

optimize: PROFILING_INFO += $(PROFDATA)
optimize: CFLAGS  += -fprofile-use=$(testdir)
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
