Srcs := $(wildcard *.c)
Outs := $(patsubst %.c, %.o, $(Srcs))
BIN := server
CC := gcc
CFLAGS = -w -g -lssl -lcrypto -lmysqlclient -pthread

ALL: $(Outs)
	$(CC) -o $(BIN) $^ $(CFLAGS)
	$(RM) $(Outs)  # 删除所有 .o 文件

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

.PHONY: clean rebuild ALL

clean:
	$(RM) $(Outs) $(BIN)
rebuild: clean ALL

