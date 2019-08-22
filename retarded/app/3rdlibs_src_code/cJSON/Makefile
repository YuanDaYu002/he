
TARGET=libcJSON.a
CC=arm-himix100-linux-gcc

all:
	$(CC) -o  cJSON.o  -c cJSON.c 
	ar -cr $(TARGET) cJSON.o  	

.PHONY: clean

clean:
	rm -f 	*.o
	rm -f	$(TARGET)

