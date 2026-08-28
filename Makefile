CC = cc
CXX = g++
CPPFLAGS += -I .

SOURCES = $(wildcard *.cpp)

.PHONY: all
all: nandcomp assembler/assembler sim

.PHONY: debug
debug: CXXFLAGS += -g
debug: CFLAGS += -g
debug: clean all

nandcomp: $(SOURCES)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $^ -o $@

assembler/assembler: assembler/assembler.cpp misc.cpp

sim: sim.c

.PHONY: clean
clean:
	rm nandcomp assembler/assembler sim || :
