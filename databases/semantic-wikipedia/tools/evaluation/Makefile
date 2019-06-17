include ../../Makefile
HEADERS  = $(wildcard *.h)
HEADERS  += $(wildcard ../../utils/*.h)

do-build: SemanticWikipediaEvaluationMain SemanticWikipediaQueryTest

do-test: SemanticWikipediaQueryTest
	./SemanticWikipediaQueryTest

do-clean:
	rm -f *.o 
	rm -f SemanticWikipediaEvaluationMain GetContextExcerpts
	rm -f gtest*
	rm -f core

SemanticWikipediaEvaluationMain: SemanticWikipediaEvaluationMain.cpp SemanticWikipediaEvaluation.o SemanticWikipediaQuery.o $(GLOBAL_OBJECTS) $(HEADERS)
	$(CXX) -o $@ SemanticWikipediaEvaluationMain.cpp SemanticWikipediaEvaluation.o SemanticWikipediaQuery.o $(GLOBAL_OBJECTS) -lexpat -L/home/buchholb/curl/lib/ -lcurl
	
SemanticWikipediaQueryTest: SemanticWikipediaQueryTest.cpp SemanticWikipediaQuery.o $(LIBGTEST_A) $(GLOBAL_OBJECTS) $(HEADERS)
	$(CXX_TEST) -o $@ SemanticWikipediaQueryTest.cpp SemanticWikipediaQuery.o $(GLOBAL_OBJECTS) -lexpat -L/home/buchholb/curl/lib -lcurl $(TEST_INCLUDE) 

SemanticWikipediaQuery.o: SemanticWikipediaQuery.cpp SemanticWikipediaQuery.h $(HEADERS)
	$(CXX) -I /home/buchholb/curl/include -c $<
	
CSOBJECTS = ../../codebase/server/Globals.o ../../codebase/server/HYBCompleter.o \
          ../../codebase/server/IndexBase.o ../../codebase/server/History.o ../../codebase/server/Vocabulary.o ../../codebase/server/codes.o ../../codebase/server/nrutil.o \
          ../../codebase/server/Separator.o ../../codebase/server/Query.o ../../codebase/server/QueryParameters.o ../../codebase/server/QueryResult.o ../../codebase/server/Completions.o \
          ../../codebase/server/SemanticQuery.o ../../codebase/server/SemanticQueryResult.o ../../codebase/server/SemanticQueryParameters.o \
          ../../codebase/server/HYBIndex.o ../../codebase/server/WordsFile.o ../../codebase/server/Vector.o ../../codebase/server/INVIndex.o \
          ../../codebase/server/ConcurrentLog.o ../../codebase/server/DocsDB.o ../../codebase/server/Document.o \
          ../../codebase/server/ExcerptsGenerator.o ../../codebase/server/ExcerptsGenerator.SemanticSearch.o ../../codebase/server/CompletionServer.o \
          ../../codebase/server/CompleterBase.o ../../codebase/server/CompleterBase.Intersect.o ../../codebase/server/CompleterBase.SemanticSearch.o ../../codebase/server/CompleterBase.TopK.o \
          ../../codebase/server/CompleterBase.Join.o ../../codebase/server/CompleterBase.FuzzySearch.o ../../codebase/server/CompleterBase.SynonymSearch.o \
          ../../codebase/server/WordRange.o ../../codebase/server/WordList.o \
          ../../codebase/utility/StringConverter.o ../../codebase/utility/WkSupport.o \
          ../../codebase/utility/TimerStatistics.o ../../codebase/utility/XmlToJson.o \
          ../../codebase/server/ZipfCompressionAlgorithm.o ../../codebase/fuzzysearch/FuzzySearcher.o
 
CSLIBS = -L $(CS_CODE_DIR)/fuzzysearch -L $(CS_CODE_DIR)/synonymsearch -lpthread -lz -lfuzzysearch -lsynonymsearch -static
          
GetContextExcerpts: GetContextExcerptsMain.cpp $(HEADERS) $(GLOBAL_OBJECTS)
	$(CXX) -DSTL_VECTOR -o $@ $^ $(CSOBJECTS) $(GLOBAL_OBJECTS) $(CSLIBS)
	
getContextExcerpts: GetContextExcerpts
		./GetContextExcerpts -i testqueries -o test.out -x /nfs/raid1/buchholb/semantic-wikipedia-data/semantic-wikipedia-full-new