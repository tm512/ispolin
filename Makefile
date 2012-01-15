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

$(MODSOUT):
	@mkdir -p modules
	cd $(patsubst modules/%.so,src/modules/%,$@); \
	make CC=$(CC) LD=$(LD) OPT=$(OPT) DBG=$(DBG) BASEDIR=$(shell pwd) OUT=$(shell pwd)/$@ \
	OBJDIR=$(shell pwd)/$(patsubst modules/%.so,$(MODOBJDIR)/%,$@) CFLAGS=$(CFLAGS)
	@cd ../..

$(OBJDIR)/%.o: src/%.c $(HDR)
	@mkdir -p $(OBJDIR)
	$(CC) -O$(OPT) -g$(DBG) $(CFLAGS) -c $< -o $@

$(OUT): $(OBJ)
	$(LD) -ldl -rdynamic $(OBJ) -o $(OUT)

clean:
	@rm -rf $(OBJDIR) $(OUT) $(MODSOUT)
