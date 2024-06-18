# SRCS:=$(wildcard *.c)
# OBJS:=$(patsubst %.c, %, $(SRCS))
# CC:=gcc
# FLAGS:=-g
# LIBS:=-lcrypt

# ALL:$(OBJS)

# %:%.c
# 	$(CC)  $^ -o $@ $(FLAGS) $(LIBS)

# clean:
# 	rm -rf $(OBJS)

SRCS := $(wildcard *.c)
OBJS := $(patsubst %.c, %.o, $(SRCS))
CC := gcc
FLAGS := -g
LIBS := -lcrypt

ALL: client

client: $(OBJS)
	$(CC) $(OBJS) -o $@ $(FLAGS) $(LIBS)

%.o: %.c
	$(CC) -c $< -o $@ $(FLAGS)

clean:
	rm -rf $(OBJS) client

