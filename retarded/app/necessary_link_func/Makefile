
CC=arm-himix100-linux-gcc
AR=arm-himix100-linux-ar
TARGET=libnecessary.a

SRCS=$(wildcard *.c)
OBJS=$(patsubst %.c,%.o,$(SRCS))

all:$(TARGET)


$(TARGET):$(OBJS)
	$(AR) -rc $@ $^
%.o:%.c
	@$(CC) -o $@ -c $<
 
PHONY: clean 

clean:
	rm -rf $(OBJS)
	rm -rf $(TARGET)
