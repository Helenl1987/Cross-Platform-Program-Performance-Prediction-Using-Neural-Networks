PAPIINC = ../..
PAPILIB = ../../libpapi.a
CC = icpc
CFLAGS += -I$(PAPIINC)
CFLAGS += -std=c++11
OS = $(shell uname)

TARGETS_NTHD = azulejosDK beautifulbridgesDK checksDK circularDK deadendDK directingrainfallDK firstofhernameDK hobsonstrainsDK karelDK minigolfDK trafficblightsDK 

TARGETS = $(TARGETS_NTHD) $(TARGETS_PTHREAD) 
LDFLAGS += $(PAPILIB)

all: $(TARGETS) 

$(TARGETS_NTHD):  %:%.o 
	$(CC) -o $@ $(CFLAGS) $^  $(LDFLAGS)

clean:
	$(RM) *.o $(TARGETS) 

