include ../Makefile

# The boost libs have a -mt ending on Cygwin (and in older Linux versions, too).
ifeq ($(wildcard /usr/lib/libboost_system-mt.dll*),)
  LIBS_INCLUDED += -L$(CS_CODE_DIR)/fuzzysearch -L$(CS_CODE_DIR)/synonymsearch -lpthread -lboost_system -lz -lfuzzysearch -lsynonymsearch
else
  LIBS_INCLUDED += -L$(CS_CODE_DIR)/fuzzysearch -L$(CS_CODE_DIR)/synonymsearch -lpthread -lboost_system-mt.dll -lz -lfuzzysearch -lsynonymsearch
endif

# NOTE(bast, 26-01-2017): -static does not work with getaddrinfo anymore.
# LIBS_INCLUDED += -static

# STXXL is used only for experimental stuff.
STXXL_COMPILER_OPTIONS = -DSORT_OPT_PREFETCHING -DUSE_MALLOC_LOCK -DCOUNT_WAIT_TIME -I/KM/ir/stxxl/ \
			 -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE \
			 -DSTXXL_USE_POSIX_MEMALIGN_ALLOC -D_XOPEN_SOURCE=600 -DSTXXL_DIRECT_IO_OFF -O3
STXXL_LINKER_OPTIONS = -L/KM/ir/stxxl/lib/ -lstxxl -lpthread

HEADERS = $(wildcard *.h)

OBJECTS = Globals.o HYBCompleter.o \
          IndexBase.o History.o Vocabulary.o codes.o nrutil.o \
          Separator.o Query.o QueryParameters.o QueryResult.o Completions.o \
          HYBIndex.o WordsFile.o Vector.o INVIndex.o \
          ConcurrentLog.o DocsDB.o Document.o CustomScorer.o \
          ExcerptsGenerator.o CompletionServer.o \
          CompleterBase.o CompleterBase.Intersect.o CompleterBase.TopK.o \
          CompleterBase.Join.o CompleterBase.FuzzySearch.o CompleterBase.SynonymSearch.o \
          CompleterBase.Join.o \
          WordRange.o WordList.o \
          ../utility/StringConverter.o ../utility/WkSupport.o \
          ../utility/TimerStatistics.o ../utility/XmlToJson.o \
          ZipfCompressionAlgorithm.o ../fuzzysearch/FuzzySearcher.o
BINARIES = startCompletionServer buildIndex buildDocsDB answerQueries
LIBS = libcompletesearch

# Rules to build individual files.

# TODO: There is a big dependency bug here. If one of the libraries from the
# LIBS_INCLUDED above changes (e.g. fuzzysearch) but nothing in the server code
# has changed, than the binaries are NOT re-linked as they should.
# The right solution, I think, is not to use libraries but link to the required
# object files from the other directories. This requires writing down files from
# other directories in the dependencies though, and I am not sure right now how
# to make that work smoothly.
# NOTE: the parser Makefile has the same problem, as it links the fuzzysearch
# and synonymsearch libraries, too.
# HACK: do make relink (see codebase/Makefile) whenever such a situation occurs.

libcompletesearch: $(OBJECTS)
	rm -f $@.a
	ar cq $@.a $^

startCompletionServer: StartCompletionServer.o $(OBJECTS)
	$(CXX) -o $@ $^ $(LIBS_INCLUDED)

buildIndex: buildIndex.o $(OBJECTS)
	$(CXX) -o $@ $^ $(LIBS_INCLUDED)

buildDocsDB: buildDocsDB.o $(OBJECTS) 
	$(CXX) -o $@ $^ $(LIBS_INCLUDED)

answerQueries: answerQueries.o $(OBJECTS)
	$(CXX) -o $@ $^ $(LIBS_INCLUDED)

answerQueries.o: answerQueries.cpp $(HEADERS) CompletionServer.h
	$(CXX) -c $*.cpp $(LIBS_INCLUDED)

answerQueriesFuzzy: answerQueriesFuzzy.o $(OBJECTS)
	$(CXX) -o $@ $^ $(LIBS_INCLUDED)

answerQueriesFuzzy.o: answerQueriesFuzzy.cpp $(HEADERS) CompletionServer.h
	$(CXX) -c $*.cpp $(LIBS_INCLUDED)

enhanceExcerpts: enhanceExcerpts.o $(OBJECTS)
	$(CXX) -o $@ $^ $(LIBS_INCLUDED)

words2docs: words2docs.o $(OBJECTS)
	$(CXX) -o $@ $^ $(LIBS_INCLUDED)

# Compile and run tests.

%Test: %Test.cpp $(OBJECTS)
	$(CXX) -o $@ $^ $(LIBS_INCLUDED) -lgtest -lpthread

%Perf: %Perf.cpp $(OBJECTS)
	$(CXX) -o $@ $^ $(LIBS_INCLUDED) -lgtest -lpthread

# Install third-party software. Currently this is only gtest.

install-gtest:
	@echo
	@echo "Follow these instructions to install gtest:"
	@echo
	@echo "1. export PATH=\`pwd\`"
	@echo "2. cd third-party/src/gtest-1.4.0"
	@echo "3. ./configure --prefix=\$$PATH/third-party"
	@echo "4. make"
	@echo "5. make install"
	@echo


# Experimental binaries.

buildDocOffsets: buildDocOffsets.o 
	$(CXX) -o $@ $^

printListLengths: printListLengths.o codes.o Globals.o nrutil.o
	$(CXX) -o $@ $^

filterQueriesByVolume: filterQueriesByVolume.o codes.o Globals.o nrutil.o
	$(CXX) -o $@ $^

buildVocabulary: buildVocabulary.o Globals.o
	$(CXX) -o $@ $^

sortVeryLargeWordsFile: sortVeryLargeWordsFile.o Globals.o
	$(CXX) -o $@ $^ $(STXXL_LINKER_OPTIONS)

sortVeryLargeWordsFile.o: sortVeryLargeWordsFile.cpp
	$(CXX) $(STXXL_COMPILER_OPTIONS) -c -o $@ $^ 

sortWordidsFile: sortWordidsFile.o Globals.o
	$(CXX) -o $@ $^ $(STXXL_LINKER_OPTIONS)

sortWordidsFile.o: sortWordidsFile.cpp
	$(CXX) $(STXXL_COMPILER_OPTIONS) -c -o $@ $^ 

sortWordidsFileInPlace: sortWordidsFileInPlace.o Globals.o
	$(CXX) -o $@ $^ $(STXXL_LINKER_OPTIONS)

sortWordidsFileInPlace.o: sortWordidsFileInPlace.cpp
	$(CXX) $(STXXL_COMPILER_OPTIONS) -c -o $@ $^ 

words2wordids: words2wordids.o Globals.o codes.o
	$(CXX) -o $@ $^

showWordidsFile: showWordidsFile.o Globals.o codes.o History.o
	$(CXX) -o $@ $^

enhanceExcerptsBySynonyms: enhanceExcerptsBySynonyms.cpp Globals.o
	$(CXX) -o $@ $^

codes.o: codes.cpp
	$(CXX) -w -c -o $@ $<

nrutil.o: nrutil.c
	$(CXX) -w -c -o $@ $<

%.o: %.cpp $(HEADERS)
	$(CXX) -DNDEBUG -c $*.cpp

# Large variety of trials and performance tests.

time-server: time-server.cc
	$(CXX) -o $@ $^

try-ranking-templated-or-not: try-ranking-templated-or-not.cpp ../libcompletesearch.a
	$(CXX) -o $@ $^ -lpthread -lz -L .. -lcompletesearch

try-rank-words: try-rank-words.cpp ../libcompletesearch.a
	$(CXX) -o $@ $^ -lpthread -lz -L .. -lcompletesearch

try-intersect: try-intersect.cpp ../libcompletesearch.a
	$(CXX) -I .. -o $@ $< -lpthread -lz -L .. -lcompletesearch

try-alternative-rank: try-alternative-rank.cpp 
	$(CXX) -o $@ $^ 

try-linkererror: try-linkererror.cpp Globals.o $(OBJECTS) CompletionServer.o 
	$(CXX) -o $@ $^

try-adler32: try-adler32.cpp ../Globals.o
	$(CXX) -o $@ $^ 

try-utf8: try-utf8.cpp ../Globals.o ../Vocabulary.o
	$(CXX) -I .. -o $@ $^ -lpthread

try-docsDB: try-docsDB.cpp Globals.o DocsDB.o Document.o
	$(CXX) -o $@ $^ -lz

try-binary-search: try-binary-search.cpp Globals.o
	g++ -O6 -Wall -DVERSION="\"`date +"Version %d%b%y %H:%M"`\"" -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -o $@ $^ 

try-zlib: try-zlib.cpp Globals.o
	g++ -O6 -Wall -DNDEBUG -DVERSION="\"`date +"Version %d%b%y %H:%M"`\"" -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -o $@ $^ -lz

try-gzip:
	export TIME="Elpased: %E (User: %U)"; \
	for N in `seq 1 9`; do \
	  /usr/bin/time gzip -$N -c ester-large.vocabulary > xxx; \
		/usr/bin/time gunzip -c xxx > yyy; ll xxx; \
	done

try-log: try-log.cpp
	g++ -O6 -Wall -lpthread -DNDEBUG -DVERSION="\"`date +"Version %d%b%y %H:%M"`\"" -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -o $@ $^

try-facets: try-facets.cpp $(OBJECTS)
	g++ -O6 -Wall -lpthread -DNDEBUG -DVERSION="\"`date +"Version %d%b%y %H:%M"`\"" -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -o $@ $^

try-thread-overhead: try-thread-overhead.cpp
	g++ -O6 -Wall -lpthread -DVERSION="\"`date +"Version %d%b%y %H:%M"`\"" -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -o $@ $^

try-asynchronous: try-asynchronous.cpp $(OBJECTS)
	g++ -O6 -Wall -lpthread -DNDEBUG -DVERSION="\"`date +"Version %d%b%y %H:%M"`\"" -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -o $@ $^

try-exception-handling: try-exception-handling.cpp
	g++ -O6 -Wall -lpthread -DVERSION="\"`date +"Version %d%b%y %H:%M"`\"" -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -o $@ $^

try-excerpts: try-excerpts.o Globals.o
	g++ -O3 -Wall -o $@ $^

try-threads: try-threads.cpp 
	g++ -O3 -Wall -lpthread -o $@ $^

try-shared-memory: try-shared-memory.cpp 
	g++ -O3 -Wall -o $@ $^

try-client: try-client.cpp 
	g++ -O3 -Wall -o $@ $^

try-server: try-server.cpp server.o
	g++ -O3 -Wall -o $@ $^

try-fork: try-fork.cpp 
	g++ -O3 -Wall -o $@ $^

try-gethits: try-gethits.cpp $(OBJECTS)
	$(CXX) -O3 -Wall -o $@ $^

try-realloc: try-realloc.o Globals.o 
	$(CXX) -o $@ $^

try-compression-timings: try-compression-timings.o codes.o Globals.o nrutil.o
	$(CXX) -o $@ $^

try-compression-correctness: try-compression-correctness.o codes.o Globals.o nrutil.o 
	$(CXX) -o $@ $^

try-decompression: try-decompression.o codes.o Globals.o nrutil.o 
	$(CXX) -o $@ $^

try-stxxl: try-stxxl.cpp
	g++ -DSORT_OPT_PREFETCHING \
	    -DUSE_MALLOC_LOCK \
	    -DCOUNT_WAIT_TIME \
	    -I/KM/ir/stxxl/ \
	    -D_FILE_OFFSET_BITS=64 \
	    -D_LARGEFILE_SOURCE \
	    -D_LARGEFILE64_SOURCE \
	    -DSTXXL_USE_POSIX_MEMALIGN_ALLOC \
	    -D_XOPEN_SOURCE=600 \
	    -O3 \
	    -o try-stxxl try-stxxl.cpp \
	    -L/KM/ir/stxxl/lib/ -lstxxl -lpthread

%: %.cpp ../libcompletesearch.a
	$(CXX) -g -I .. -o $@ $< -lpthread -lz -L .. -lcompletesearch

# Rules for archiving the source and the binaries.

completesearch.tgz:
	mkdir tar.TMP
	mkdir tar.TMP/completesearch
	cp Makefile Doxyfile *.h *.cpp *.c tar.TMP/completesearch
	cd tar.TMP; tar -zcf ../$@ completesearch/*
	ls -lh $@
	tar -ztf $@ | head -5; echo ...; tar -ztf $@ | tail -5
	cp $@ ~/www/topsecret
	ls -lh ~/www/topsecret/$@
	rm -rf tar.TMP

completesearch.binaries.tgz: buildIndex buildDocsDB startCompletionServer
	tar -zcvf $@ $^
	cp $@ ~/www/topsecret

