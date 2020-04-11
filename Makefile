TARGET = public-key-cryptosystem
LDLIBS = -lm
CFLAGS += -std=c99 -D_DEFAULT_SOURCE -Wall 
RM = rm -f
JUNK=*.o *~ *.dSYM

.PHONY: all clean

all: $(TARGET)


clean:
	$(RM) $(TARGET) $(JUNK)