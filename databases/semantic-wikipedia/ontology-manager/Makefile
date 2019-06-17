include ../Makefile
HEADERS += $(wildcard *.h)

%.o: %.cpp $(HEADERS)
	$(CXX) -c $<
	
# -----------------------------------------------------------------------------
# Targets specific to building, testing, etc. the OntologyManager and related files.
# -----------------------------------------------------------------------------

do-build: OntologyManagerMain CreateOntologyEntityListMain OntologyManagerTest RelationTest
	
do-clean:
	rm -f *.o
	rm -f stxxl.disk
	rm -f core
	rm -f OntologyManagerMain OntologyManagerTest RelationTest SimpleWordnetCategoryDisambiguator CreateOntologyEntityListMain
	
do-test: OntologyManagerTest RelationTest
	./OntologyManagerTest 
	./RelationTest
	
OntologyManagerMain: OntologyManagerMain.cpp OntologyManager.o OntologyRelation.o OutputWriter.o SUSIOutputWriter.o RelationsOutputWriter.o RelationsOutputWriterOld.o $(GLOBAL_OBJECTS) $(HEADERS) 
	$(CXX) -o $@ OntologyManagerMain.cpp OntologyManager.o OntologyRelation.o OutputWriter.o SUSIOutputWriter.o RelationsOutputWriter.o RelationsOutputWriterOld.o $(GLOBAL_OBJECTS)
	
CreateOntologyEntityListMain: CreateOntologyEntityListMain.o $(GLOBAL_OBJECTS) $(HEADERS)
	$(CXX) -o $@ CreateOntologyEntityListMain.o $(GLOBAL_OBJECTS)
	
OntologyManagerTest: OntologyManagerTest.o OntologyManager.o OntologyRelation.o OutputWriter.o SUSIOutputWriter.o RelationsOutputWriter.o RelationsOutputWriterOld.o $(GLOBAL_OBJECTS) $(LIBGTEST_A) $(HEADERS)
	$(CXX_TEST) -o $@ OntologyManagerTest.o OntologyManager.o OntologyRelation.o OutputWriter.o SUSIOutputWriter.o RelationsOutputWriter.o RelationsOutputWriterOld.o $(GLOBAL_OBJECTS) $(TEST_INCLUDE)
	
RelationTest: RelationTest.o OntologyManager.o OntologyRelation.o $(GLOBAL_OBJECTS) $(HEADERS)
	$(CXX_TEST) -o $@ RelationTest.o OntologyManager.o OntologyRelation.o $(GLOBAL_OBJECTS) $(TEST_INCLUDE)
	

CSOBJECTS = ../codebase/server/Globals.o ../codebase/server/HYBCompleter.o ../codebase/server/CustomScorer.o \
          ../codebase/server/IndexBase.o ../codebase/server/History.o ../codebase/server/Vocabulary.o ../codebase/server/codes.o ../codebase/server/nrutil.o \
          ../codebase/server/Separator.o ../codebase/server/Query.o ../codebase/server/QueryParameters.o ../codebase/server/QueryResult.o ../codebase/server/Completions.o \
          ../codebase/server/SemanticQuery.o ../codebase/server/SemanticQueryResult.o ../codebase/server/SemanticQueryParameters.o \
          ../codebase/server/HYBIndex.o ../codebase/server/WordsFile.o ../codebase/server/Vector.o ../codebase/server/INVIndex.o \
          ../codebase/server/ConcurrentLog.o ../codebase/server/DocsDB.o ../codebase/server/Document.o \
          ../codebase/server/ExcerptsGenerator.o ../codebase/server/ExcerptsGenerator.SemanticSearch.o ../codebase/server/CompletionServer.o \
          ../codebase/server/CompleterBase.o ../codebase/server/CompleterBase.Intersect.o ../codebase/server/CompleterBase.SemanticSearch.o ../codebase/server/CompleterBase.TopK.o \
          ../codebase/server/CompleterBase.Join.o ../codebase/server/CompleterBase.FuzzySearch.o ../codebase/server/CompleterBase.SynonymSearch.o \
          ../codebase/server/WordRange.o ../codebase/server/WordList.o \
          ../codebase/utility/StringConverter.o ../codebase/utility/WkSupport.o \
          ../codebase/utility/TimerStatistics.o ../codebase/utility/XmlToJson.o \
          ../codebase/server/ZipfCompressionAlgorithm.o ../codebase/fuzzysearch/FuzzySearcher.o
 
CSLIBS = -L $(CS_CODE_DIR)/fuzzysearch -L $(CS_CODE_DIR)/synonymsearch -lpthread -lz -lfuzzysearch -lsynonymsearch -static
          
SimpleWordnetCategoryDisambiguator: SimpleWordnetCategoryDisambiguator.cpp $(GLOBAL_OBJECTS) $(HEADERS)
	$(CXX) -DSTL_VECTOR -o $@ $^ $(CSOBJECTS) $(CSLIBS)
