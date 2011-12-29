CC = gcc
LD = gcc
OPT = 0
DBG = 3
OBJDIR = obj
SRC = $(wildcard src/*.c)
HDR = $(wildcard src/*.h)
OBJ = $(patsubst src/%.c, $(OBJDIR)/%.o, $(SRC))
OUT = ispolin

.PHONY: default clean

default: $(OUT)

$(OBJDIR)/%.o: src/%.c $(HDR)
	@mkdir -p $(OBJDIR)
	$(CC) -O$(OPT) -g$(DBG) -c $< -o $@

$(OUT): $(OBJ)
	$(LD) $(OBJ) -o $(OUT)

clean:
	@rm -rf $(OBJDIR) $(OUT)
