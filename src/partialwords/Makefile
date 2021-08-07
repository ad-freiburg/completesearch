# include ../Makefile

CXX = g++ -g
CC = gcc -g
SACA_DIR = .

all:
	$(CC) -c $(SACA_DIR)/gsacak.c
	$(MAKE) -B PartialWordsMain

OBJECTS = PartialWords.o $(SACA_DIR)/gsacak.o
HEADERS = PartialWords.h

BINARIES = $(basename $(wildcard *Main.cpp))
TEST_BINARIES = $(basename $(wildcard *Test.cpp))
PERF_BINARIES = $(basename $(wildcard *Perf.cpp))
LIBS = libpartialwords.a

libpartialwords.a: PartialWords.o
	rm -f $@
	ar cq $@ $^

PartialWordsMain: PartialWordsMain.cpp $(OBJECTS)
	$(CXX) -o $@ $^ -lpthread $(LIBS_INCLUDED)

PartialWordsTest: PartialWordsTest.cpp $(OBJECTS)
	$(CXX) -o $@ $^ -lgtest -lgtest_main -lpthread $(LIBS_INCLUDED)

%.o: %.cpp $(HEADERS)
	$(CXX) -c $<
