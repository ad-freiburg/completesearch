include ../Makefile

# CXX_OPTIONS = -O9 -Wall -fexceptions -I$(CS_CODE_DIR) -Wno-deprecated
# CXX_DEFINES = -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_REENTRANT -DSTL_VECTOR
# CXX = g++ $(CXX_OPTIONS) $(CXX_DEFINES)

# NOTE(bast, 26-01-2017): -static does not work with getaddrinfo anymore.
# LIBS_INCLUDED += -static

HEADERS = $(wildcard *.h)
OBJECTS = StringDistances.o WordClusteringBuilder.o PermutedLexicon.o FastSS.o FuzzySearcher.o Utils.o
BINARIES = buildFuzzySearchClusters loadFuzzySearcher
LIBS = libfuzzysearch.a

libfuzzysearch.a: $(OBJECTS)
	rm -f $@
	ar cq $@ $^

%Test: %Test.o $(OBJECTS)
	$(CXX) -o $@ $^ $(LIBS_INCLUDED) -lgtest -lpthread
	
buildFuzzySearchClusters: BuildFuzzySearchClusters.o $(OBJECTS)
	$(CXX) -o $@ $^ $(LIBS_INCLUDED)

fuzzySearchTest: FuzzySearchTest.o $(OBJECTS)
	$(CXX) -o $@ $^ -lgtest -lpthread $(LIBS_INCLUDED)

loadFuzzySearcher: LoadFuzzySearcher.o $(OBJECTS)
	$(CXX) -o $@ $^

%.o: %.cpp $(HEADERS)
	$(CXX) -c -o $@ $<
