Srcs := $(wildcard *.c)
Outs := $(patsubst %.c, %.o, $(Srcs))
BIN := server
CC := gcc
CFLAGS := -Wall -g -pthread

ALL: $(BIN)

$(BIN): $(Outs)
	$(CC) -o $@ $^ $(CFLAGS)
	rm -f $(Outs)  # 删除所有 .o 文件

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

.PHONY: clean rebuild ALL

clean:
	$(RM) $(Outs) $(BIN)

rebuild: clean ALL

