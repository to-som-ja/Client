OUTPUTS = client
CC = gcc

all: $(OUTPUTS)

clean:
	rm -f $(OUTPUTS)

.PHONY: all clean

client: main.c Mravec.h Data.h Client.h Client.c
	$(CC) -o client main.c Client.c -pthread