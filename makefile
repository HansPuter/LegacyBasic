#
#

HEADERS = list.h token.h tokenizer.h list.h run.h
SOURCES = lbasic.c tokenizer.c list.c run.c
TARGET  = lbasic

all: lbasic

lbasic: $(HEADERS) $(SOURCES)
	gcc -Wall -ggdb $(SOURCES) -o $(TARGET) -lm

release:
	gcc -Wall -O2 $(SOURCES) -o $(TARGET) -lm

clean:
	rm -f $(TARGET)
#	rm -f test2
	
