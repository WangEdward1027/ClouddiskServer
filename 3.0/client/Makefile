SRCS:=$(wildcard *.c)
OBJS:=$(patsubst %.c, %.o, $(SRCS))
BIN:=main
CC:=gcc
FLAGS:=-g -w -lssl -lcrypto

$(BIN):$(OBJS)
	$(CC) -o $@ $^ $(FLAGS)
%.o:%.c
	$(CC) -c $^ -o $@ $(FLAGS)

clean:
	rm -rf $(OBJS) $(BIN)
