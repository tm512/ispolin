CC = gcc
LD = gcc
OPT = 0
DBG = 3
OBJDIR = obj
MODOBJDIR = $(OBJDIR)/modules
SRC = $(wildcard src/*.c)
HDR = $(wildcard src/*.h)
MODS = $(wildcard src/modules/*)
OBJ = $(patsubst src/%.c,$(OBJDIR)/%.o,$(SRC))
OUT = ispolin
MODSOUT = $(patsubst src/modules/%,modules/%.so,$(MODS))

.PHONY: default clean

default: $(OUT) $(MODSOUT)

# System specific hacks!!! (*vomit*)
ifneq ($(strip $(shell $(CC) -v 2>&1 | grep -i "freebsd")),)
CFLAGS=-I/usr/local/include/ -I/usr/local/include/lua51
LDFLAGS=-L/usr/local/lib/ -L/usr/local/lib/lua51 -lm -llua
endif

ifneq ($(strip $(shell $(CC) -v 2>&1 | grep -Ei "netbsd|dragonfly")),)
CFLAGS=-I/usr/pkg/include
LDFLAGS=-L/usr/pkg/lib/ -lm -llua
endif

ifneq ($(strip $(shell $(CC) -v 2>&1 | grep -i "openbsd")),)
CFLAGS=-I/usr/local/include
LDFLAGS=-L/usr/local/lib -lm -llua -export-dynamic
endif

ifneq ($(strip $(shell $(CC) -v 2>&1 | grep -i "linux")),)
LDFLAGS=-ldl -llua
endif

src/version.h:
	@echo "Generating version.h..."
	@./version.sh

$(MODSOUT): $(wildcard $(patsubst src/modules/%,src/modules/%/,$(MODS))/*.[ch]) src/version.h
	@mkdir -p modules
	@cd $(patsubst modules/%.so,src/modules/%,$@); \
	$(MAKE) CC=$(CC) LD=$(LD) OPT=$(OPT) DBG=$(DBG) BASEDIR=$(shell pwd) OUT=$(shell pwd)/$@ \
	OBJDIR=$(shell pwd)/$(patsubst modules/%.so,$(MODOBJDIR)/%,$@) CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)"
	@cd ../..

$(OBJDIR)/%.o: src/%.c $(HDR) src/version.h
	@mkdir -p $(OBJDIR)
	$(CC) -O$(OPT) -g$(DBG) $(CFLAGS) -c $< -o $@

$(OUT): $(OBJ)
	$(LD) $(LDFLAGS) -rdynamic $(OBJ) -o $(OUT)

clean:
	@rm -rf $(OBJDIR) $(OUT) $(MODSOUT) src/version.h
