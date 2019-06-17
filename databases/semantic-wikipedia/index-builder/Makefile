include ../Makefile
include $(STXXL_CONFIG)

CXX          += $(STXXL_CPPFLAGS) -Wno-deprecated

HEADERS  += $(wildcard *.h)
HEADERS  += ../server/Identifiers.h ../server/IndexMetaData.h

%.o: %.cpp $(HEADERS)
	$(CXX) -c $<
	
OBJECTS  = VocabularyBuilder.o IndexBuilderBase.o Extractors.o SemsearchIndexBuilder.o PlainSerializer.o PlainOntologySerializer.o
BINARIES = VocabularyBuilderMain IndexBuilderMain 
TESTBINARIES = VocabularyBuilderTest ExtractorsTest IndexBuilderComparatorsTest IndexBuilderBaseTest PlainSerializerTest PlainOntologySerializerTest

do-build: $(BINARIES)

do-test: $(TESTBINARIES)
	./VocabularyBuilderTest
	./IndexBuilderComparatorsTest
	./ExtractorsTest
	./IndexBuilderBaseTest
	./PlainSerializerTest
	./PlainOntologySerializerTest
	
do-clean:
	rm -f core
	rm -f *.o $(BINARIES) $(TESTBINARIES)
	rm -f __tmp.test.serializer-out.binary
	rm -f __tmp.test.fulltext.ascii
	rm -f __tmp.test.ontology.ascii
	rm -f __tmp.test.words-unsorted.ascii
	rm -f stxxl.errlog
	rm -f stxxl.log

IndexBuilderMain: IndexBuilderMain.o IndexBuilderBase.o Extractors.o PlainOntologySerializer.o SemsearchIndexBuilder.o PlainSerializer.o $(GLOBAL_OBJECTS) $(HEADERS)
	$(CXX) -o $@ IndexBuilderMain.o IndexBuilderBase.o Extractors.o PlainOntologySerializer.o SemsearchIndexBuilder.o PlainSerializer.o $(GLOBAL_OBJECTS) $(STXXL_LDLIBS)

VocabularyBuilderMain: VocabularyBuilderMain.o VocabularyBuilder.o $(GLOBAL_OBJECTS) $(HEADERS) 
	$(CXX) -o $@ VocabularyBuilderMain.o VocabularyBuilder.o $(GLOBAL_OBJECTS)
	
VocabularyBuilderTest: VocabularyBuilderTest.o VocabularyBuilder.o $(GLOBAL_OBJECTS) $(HEADERS) $(LIBGTEST_A)
	$(CXX_TEST) -o $@ VocabularyBuilderTest.o VocabularyBuilder.o $(GLOBAL_OBJECTS) $(TEST_INCLUDE)

IndexBuilderBaseTest: IndexBuilderBaseTest.o IndexBuilderBase.o Extractors.o PlainOntologySerializer.o PlainSerializer.o $(GLOBAL_OBJECTS) $(HEADERS) $(LIBGTEST_A)
	$(CXX_TEST) -o $@ IndexBuilderBaseTest.o IndexBuilderBase.o Extractors.o PlainOntologySerializer.o PlainSerializer.o $(GLOBAL_OBJECTS) $(TEST_INCLUDE) $(STXXL_LDLIBS)
	
ExtractorsTest: ExtractorsTest.cpp Extractors.o $(GLOBAL_OBJECTS) $(HEADERS) $(LIBGTEST_A)
	$(CXX_TEST) -o $@ $< Extractors.o $(GLOBAL_OBJECTS) $(TEST_INCLUDE) $(STXXL_LDLIBS)
	
IndexBuilderComparatorsTest: IndexBuilderComparatorsTest.cpp $(GLOBAL_OBJECTS) $(HEADERS) $(LIBGTEST_A)
	$(CXX_TEST) -o $@ $< $(GLOBAL_OBJECTS) $(TEST_INCLUDE)

PlainOntologySerializerTest: PlainOntologySerializerTest.cpp PlainOntologySerializer.o $(GLOBAL_OBJECTS) $(HEADERS) $(LIBGTEST_A)
	$(CXX_TEST) -o $@ $< PlainOntologySerializer.o $(GLOBAL_OBJECTS) $(TEST_INCLUDE) $(STXXL_LDLIBS)
		
PlainSerializerTest: PlainSerializerTest.cpp PlainSerializer.o $(GLOBAL_OBJECTS) $(HEADERS) $(LIBGTEST_A)
	$(CXX_TEST) -o $@ $< PlainSerializer.o $(GLOBAL_OBJECTS) $(TEST_INCLUDE) $(STXXL_LDLIBS)

	
