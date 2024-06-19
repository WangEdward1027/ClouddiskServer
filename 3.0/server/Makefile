Srcs := $(wildcard *.c)
Outs := $(patsubst %.c, %.o, $(Srcs))
BIN:=CloudiskServer
CC := gcc
CFLAGS = -w -g

ALL: $(Outs) 
	$(CC) -o $(BIN) $^ $(CFLAGS)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

.PHONY: clean rebuild ALL

clean:
	$(RM) $(Outs) $(BIN)
rebuild: clean ALL
