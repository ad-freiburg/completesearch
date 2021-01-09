include ../../Makefile

LIBS_INCLUDED += -L $(CS_CODE_DIR)/fuzzysearch -L $(CS_CODE_DIR)/synonymsearch -lpthread -lz -lfuzzysearch -lsynonymsearch -lboost_thread 
HEADERS  = $(wildcard *.h)
OBJECTS    = ../Globals.o ../HYBCompleter.o \
	     ../IndexBase.o ../History.o ../Vocabulary.o ../codes.o ../nrutil.o \
             ../Separator.o ../Query.o ../QueryParameters.o ../QueryResult.o ../Completions.o \
    ../SemanticQuery.o ../SemanticQueryResult.o ../SemanticQueryParameters.o \
             ../HYBIndex.o ../WordsFile.o ../Vector.o ../INVIndex.o \
             ../ConcurrentLog.o ../DocsDB.o ../Document.o \
             ../ExcerptsGenerator.o ../ExcerptsGenerator.SemanticSearch.o ../CompletionServer.o \
             ../CompleterBase.o ../CompleterBase.Intersect.o ../CompleterBase.SemanticSearch.o ../CompleterBase.TopK.o \
             ../CompleterBase.Join.o ../CompleterBase.FuzzySearch.o ../CompleterBase.SynonymSearch.o \
             ../WordRange.o ../WordList.o \
	     ../../utility/StringConverter.o ../../utility/WkSupport.o \
	     ../../utility/TimerStatistics.o \
             ../ZipfCompressionAlgorithm.o ../../fuzzysearch/FuzzySearcher.o \
      ../../utility/XmlToJson.o \
         ./CompleterBase.PerformanceEvaluator.o ./Utilities.o \
	      ./CompleterBase.IntersectAlgorithms.o ./CompleterBase.TestStatistics.o
        

%Perf: %Perf.cpp $(OBJECTS)
	$(CXX) -o $@ $^ $(LIBS_INCLUDED) -lgtest

%Test: %Test.cpp $(OBJECTS)
	$(CXX) -o $@ $^ $(LIBS_INCLUDED) -lgtest
