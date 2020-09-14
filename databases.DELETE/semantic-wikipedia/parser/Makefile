include ../Makefile
HEADERS += $(wildcard *.h)

# -----------------------------------------------------------------------------
# General purpose targets
# -----------------------------------------------------------------------------

%.o: %.cpp $(HEADERS)
	$(CXX) -c $<

# -----------------------------------------------------------------------------
# Targets specific to building, testing, etc. the SemanticWikipediaParser.
# -----------------------------------------------------------------------------

do-build: SemanticWikipediaParserTest SemanticWikipediaParserMain

do-test: SemanticWikipediaParserTest EntityAwareWriterTest EntityRepositoryTest
	./SemanticWikipediaParserTest
	./EntityAwareWriterTest
	./EntityRepositoryTest

do-clean:
	rm -f *.o
	rm -f core  
	rm -f SemanticWikipediaParserMain SemanticWikipediaParserTest EntityAwareWriterTest EntityRepositoryTest gmon.out
	rm -f gtest*

SemanticWikipediaParserMain: SemanticWikipediaParserMain.o ../codebase/parser/XmlParser.cpp SemanticWikipediaParser.o EntityAwareWriter.o EntityRepository.o $(GLOBAL_OBJECTS) $(HEADERS) 
	$(CXX) -o $@ SemanticWikipediaParserMain.o ../codebase/parser/XmlParser.cpp SemanticWikipediaParser.o EntityAwareWriter.o EntityRepository.o $(GLOBAL_OBJECTS) -lexpat 

SemanticWikipediaParserTest: SemanticWikipediaParserTest.o SemanticWikipediaParser.o EntityAwareWriter.o EntityRepository.o ../codebase/parser/XmlParser.cpp $(LIBGTEST_A) $(HEADERS) $(GLOBAL_OBJECTS)
	$(CXX_TEST) -o $@ SemanticWikipediaParserTest.o SemanticWikipediaParser.o EntityAwareWriter.o EntityRepository.o ../codebase/parser/XmlParser.cpp $(GLOBAL_OBJECTS) -lexpat $(TEST_INCLUDE)

EntityAwareWriterTest: EntityAwareWriterTest.o EntityAwareWriter.o EntityRepository.o $(LIBGTEST_A) $(HEADERS) $(GLOBAL_OBJECTS)
	$(CXX_TEST) -o $@ EntityAwareWriterTest.o EntityAwareWriter.o EntityRepository.o $(GLOBAL_OBJECTS) $(TEST_INCLUDE)
	
EntityRepositoryTest: EntityRepositoryTest.o EntityRepository.o $(LIBGTEST_A) $(HEADERS) $(GLOBAL_OBJECTS)
	$(CXX_TEST) -o $@ EntityRepositoryTest.o EntityRepository.o $(GLOBAL_OBJECTS) $(TEST_INCLUDE)