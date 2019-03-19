RM = -rm -rf

CC = arm-hisiv500-linux-gcc

SRCS = $(wildcard *.c)
SOS = $(patsubst %.c,%.so,$(SRCS))

all: $(SOS)

$(SOS): %.so : %.c
	@$(CC) -mlong-calls -nostdlib $< -fPIC -shared -o $@

clean:
	@$(RM) $(SOS)

.PHONY: all clean
