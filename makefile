#
#

HEADERS = list.h token.h 
SOURCES = lbasic.c
TARGET  = lbasic

all: lbasic

lbasic: $(HEADERS) $(SOURCES)
	gcc -Wall -O2 $(SOURCES) -o $(TARGET)

debug:
	gcc -Wall -ggdb $(SOURCES) -o $(TARGET)

clean:
	rm -f $(TARGET)
#	rm -f test2
	
