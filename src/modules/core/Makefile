SRC = $(wildcard *.c)
HDR = $(wildcard *.h)
OBJ = $(patsubst %.c, $(OBJDIR)/%.o, $(SRC))

.PHONY: default

default: $(OUT)

$(OBJDIR)/%.o: %.c $(HDR)
	@mkdir -p $(OBJDIR)
	$(CC) -fPIC -I$(BASEDIR)/src -O$(OPT) -g$(DBG) $(CFLAGS) -c $< -o $@

$(OUT): $(OBJ)
	$(LD) -shared $(OBJ) -o $(OUT)
