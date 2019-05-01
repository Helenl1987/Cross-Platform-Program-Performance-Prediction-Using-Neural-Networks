PAPIINC = ../..
PAPILIB = ../../libpapi.a
CC = icpc
CFLAGS += -I$(PAPIINC)
CFLAGS += -std=c++11
OS = $(shell uname)

TARGETS_NTHD = artDK asteroidDK cateringDK cheeseDK evolutionDK keyboardDK pipeDK qanatDK shipDK toursDK weatherDK

TARGETS = $(TARGETS_NTHD) $(TARGETS_PTHREAD) 
LDFLAGS += $(PAPILIB)

all: $(TARGETS) 

$(TARGETS_NTHD):  %:%.o 
	$(CC) -o $@ $(CFLAGS) $^  $(LDFLAGS)

clean:
	$(RM) *.o $(TARGETS) 

