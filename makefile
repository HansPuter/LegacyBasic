#
#CC=tcc
CC=gcc

HEADERS = list.h token.h tokenizer.h list.h run.h
SOURCES = lbasic.c tokenizer.c list.c run.c
TARGET  = lbasic

all: lbasic

lbasic: $(HEADERS) $(SOURCES)
	$(CC) -Wall -ggdb $(SOURCES) -o $(TARGET) -lm

release:
	$(CC) -Wall -O2 $(SOURCES) -o $(TARGET) -lm

clean:
	rm -f $(TARGET)
#	rm -f test2
	
