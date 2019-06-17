include ../Makefile
HEADERS  += $(wildcard *.h)
OBJECTS  = Index.o Vocabulary.o Excerpt.o Engine.o Query.o QueryParameters.o QueryResult.o QueryTreeNode.o Disjunct.o QueryTriple.o ExcerptOffsetCollection.o Server.o
BINARIES = ServerMain IndexPerformanceTest QueryLogicPerformanceTest QueryEndToEndPerformanceTest ExcerptsOffsetsMain
TESTBINARIES = EntityListTest IdentifiersTest IndexTest IndexMetaDataTest VocabularyTest ExcerptTest EngineTest QueryTripleTest QueryTest

%.o: %.cpp $(HEADERS)
	$(CXX) -c $<	
	
do-build: $(OBJECTS) $(BINARIES)

do-test: $(TESTBINARIES)
	./EntityListTest
	./IdentifiersTest
	./IndexMetaDataTest
	./VocabularyTest
	./IndexTest
	./ExcerptTest
	./EngineTest
	./QueryTripleTest
	./QueryTest

do-clean:
	rm -f core
	rm -f *.o $(BINARIES) $(TESTBINARIES)

do-perf-test: $(BINARIES)
	./IndexPerformanceTest -o $(DATA_DIRECTORY)/ontology -t $(DB)-full
	./QueryLogicPerformanceTest -o $(DATA_DIRECTORY)/ontology -t $(DB)-full
	./QueryEndToEndPerformanceTest -o $(DATA_DIRECTORY)/ontology -t $(DB)-full		

ServerMain: ServerMain.cpp $(OBJECTS) $(GLOBAL_OBJECTS) $(HEADERS)
	$(CXX) -o $@ ServerMain.cpp $(OBJECTS) $(GLOBAL_OBJECTS)

ExcerptsOffsetsMain: ExcerptsOffsetsMain.cpp ExcerptOffsetCollection.o
	$(CXX) -o $@ ExcerptsOffsetsMain.cpp ExcerptOffsetCollection.o $(GLOBAL_OBJECTS)

IndexPerformanceTest: IndexPerformanceTest.cpp Index.o Engine.o Vocabulary.o Excerpt.o ExcerptOffsetCollection.o $(GLOBAL_OBJECTS) $(HEADERS)
	$(CXX) -o $@ IndexPerformanceTest.cpp Index.o Engine.o Vocabulary.o Excerpt.o ExcerptOffsetCollection.o $(GLOBAL_OBJECTS)

QueryLogicPerformanceTest: QueryLogicPerformanceTest.cpp $(OBJECTS) $(GLOBAL_OBJECTS) $(HEADERS)
	$(CXX) -o $@ QueryLogicPerformanceTest.cpp $(OBJECTS) $(GLOBAL_OBJECTS) 

QueryEndToEndPerformanceTest: QueryEndToEndPerformanceTest.cpp $(OBJECTS) $(GLOBAL_OBJECTS) $(HEADERS)
	$(CXX) -o $@ QueryEndToEndPerformanceTest.cpp $(OBJECTS) $(GLOBAL_OBJECTS) 
	
IndexTest: IndexTest.cpp Index.o Vocabulary.o Excerpt.o ExcerptOffsetCollection.o $(GLOBAL_OBJECTS) $(HEADERS) $(LIBGTEST_A)
	$(CXX_TEST) -o $@ IndexTest.cpp Index.o Vocabulary.o Excerpt.o ExcerptOffsetCollection.o $(GLOBAL_OBJECTS) $(TEST_INCLUDE)
	
EngineTest: EngineTest.cpp Engine.o Excerpt.o $(GLOBAL_OBJECTS) $(HEADERS) $(LIBGTEST_A)
	$(CXX_TEST) -o $@ EngineTest.cpp Engine.o Excerpt.o $(GLOBAL_OBJECTS) $(TEST_INCLUDE)

VocabularyTest: VocabularyTest.cpp Vocabulary.o $(GLOBAL_OBJECTS) $(HEADERS) $(LIBGTEST_A)
	$(CXX_TEST) -o $@ VocabularyTest.cpp Vocabulary.o $(GLOBAL_OBJECTS) $(TEST_INCLUDE)

IndexMetaDataTest: IndexMetaDataTest.cpp $(GLOBAL_OBJECTS) $(HEADERS) $(LIBGTEST_A)
	$(CXX_TEST) -o $@ IndexMetaDataTest.cpp $(GLOBAL_OBJECTS) $(TEST_INCLUDE)
	
EntityListTest: EntityListTest.cpp $(GLOBAL_OBJECTS) $(HEADERS) $(LIBGTEST_A)
	$(CXX_TEST) -o $@ EntityListTest.cpp $(GLOBAL_OBJECTS) $(TEST_INCLUDE)

ExcerptTest: ExcerptTest.cpp Excerpt.o $(GLOBAL_OBJECTS) $(HEADERS) $(LIBGTEST_A)	
	$(CXX_TEST) -o $@ ExcerptTest.cpp Excerpt.o $(GLOBAL_OBJECTS) $(TEST_INCLUDE)

QueryTest: QueryTest.cpp $(OBJECTS) $(GLOBAL_OBJECTS) $(HEADERS) $(LIBGTEST_A)	
	$(CXX_TEST) -o $@ QueryTest.cpp $(OBJECTS) $(GLOBAL_OBJECTS) $(TEST_INCLUDE)
	
QueryTripleTest: QueryTripleTest.cpp $(OBJECTS) $(GLOBAL_OBJECTS) $(HEADERS) $(LIBGTEST_A)	
	$(CXX_TEST) -o $@ QueryTripleTest.cpp $(OBJECTS) $(GLOBAL_OBJECTS) $(TEST_INCLUDE)
	
%Test: %Test.cpp $(HEADERS)
	$(CXX_TEST) -o $@ $< $(TEST_INCLUDE)
	
 