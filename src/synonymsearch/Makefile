include ../Makefile

OBJECTS = SynonymDictionary.o
HEADERS = SynonymDictionary.h

BINARIES = 
TEST_BINARIES = $(basename $(wildcard *Test.cpp))
PERF_BINARIES = $(basename $(wildcard *Perf.cpp))
LIBS = libsynonymsearch.a

libsynonymsearch.a: SynonymDictionary.o
	rm -f $@
	ar cq $@ $^

SynonymDictionaryTest: SynonymDictionaryTest.cpp $(OBJECTS)
	$(CXX) -o $@ $^ -lgtest -lgtest_main -lpthread $(LIBS_INCLUDED)

SynonymDictionaryPerf: $(OBJECTS) SynonymDictionaryPerf.o
	$(CXX) -o $@ $^

%.o: %.cpp $(HEADERS)
	$(CXX) -DNDEBUG -c $<
