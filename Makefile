CC = g++
CPPFLAGS += -I .

SOURCES = $(wildcard *.cpp)

all: nandcomp assembler/assembler sim

debug: CXXFLAGS += -g
debug: clean all

nandcomp: $(SOURCES)
	$(CC) $(CXXFLAGS) $(CPPFLAGS) $^ -o $@

assembler/assembler: assembler/assembler.cpp misc.cpp

sim: CC = cc
sim: sim.c

clean:
	rm nandcomp assembler/assembler sim || :
