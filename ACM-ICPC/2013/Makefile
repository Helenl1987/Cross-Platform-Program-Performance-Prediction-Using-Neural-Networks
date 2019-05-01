PAPIINC = ../..
PAPILIB = ../../libpapi.a
CC = icpc
CFLAGS += -I$(PAPIINC)
CFLAGS += -std=c++11
OS = $(shell uname)

TARGETS_NTHD = treeDK pollutionDK pirateDK matryoshkaDK maptilesDK lowDK harvardDK assemblyDK congestDK betterDK 

TARGETS = $(TARGETS_NTHD) $(TARGETS_PTHREAD) 
LDFLAGS += $(PAPILIB)

all: $(TARGETS) 

$(TARGETS_NTHD):  %:%.o 
	$(CC) -o $@ $(CFLAGS) $^  $(LDFLAGS)

clean:
	$(RM) *.o $(TARGETS) 

