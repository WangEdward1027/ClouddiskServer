SRCS := $(wildcard *.c)
OBJS := $(patsubst %.c, %.o, $(SRCS))
CC := gcc
FLAGS := -g -w
LIBS :=  -lssl -lcrypto

ALL: client

client: $(OBJS)
	$(CC) $(OBJS) -o $@ $(FLAGS) $(LIBS)
	rm -rf $(OBJS)  # 这行是新添加的，用于清除所有 .o 文件

%.o: %.c
	$(CC) -c $< -o $@ $(FLAGS)

clean:
	rm -rf $(OBJS) client

