include ../Makefile
HEADERS  += $(wildcard *.h)

do-build: buildRedirectMapBuilder buildExtractor
	$(MAKE) -C evaluation/ do-build

do-test:
	$(MAKE) -C evaluation/ do-test  

do-clean: cleanRedirectMapBuilder cleanExtractor
	$(MAKE) -C evaluation/ do-clean
	rm -f core
	rm -f *.o
	rm -f GraphTest PathsInCategoryGraphMain
 
# -----------------------------------------------------------------------------
# General purpose targets
# -----------------------------------------------------------------------------

%.o: %.cpp $(HEADERS)
	$(CXX) -c $<

PathsInCategoryGraphMain: PathsInCategoryGraphMain.cpp Graph.o $(HEADERS)
	$(CXX) -o $@ PathsInCategoryGraphMain.cpp Graph.o

GraphTest: GraphTest.cpp Graph.o $(HEADERS) $(GLOBAL_OBJECTS) $(LIBGTEST_A)	
	$(CXX_TEST) -o $@ GraphTest.cpp Graph.o $(GLOBAL_OBJECTS) $(TEST_INCLUDE)	
			
# -----------------------------------------------------------------------------
# Targets specific to building, testing, etc. the RedirectMapBuilder.
# -----------------------------------------------------------------------------

buildRedirectMapBuilder: RedirectMapBuilderMain
	
cleanRedirectMap: 
	rm -f *.redirect-map-unsorted
	rm -f *.redirect-map
	
cleanRedirectMapBuilder:
	rm -f RedirectMapBuilder.o 
	rm -f RedirectMapBuilderMain

RedirectMapBuilderMain: RedirectMapBuilderMain.cpp RedirectMapBuilder.o $(HEADERS) $(GLOBAL_OBJECTS)
	$(CXX) -o $@ RedirectMapBuilderMain.cpp RedirectMapBuilder.o $(GLOBAL_OBJECTS) -lexpat

	
# -----------------------------------------------------------------------------
# Targets specific to building, testing, etc. the SemanticWikipediaExtractor and related files.
# -----------------------------------------------------------------------------

buildExtractor: SemanticWikipediaExtractorMain
	
cleanExtractor:
	rm -f SemanticWikipediaExtractor.o 
	rm -f SemanticWikipediaExtractorMain


SemanticWikipediaExtractorMain: SemanticWikipediaExtractorMain.cpp SemanticWikipediaExtractor.o $(HEADERS) $(GLOBAL_OBJECTS)
	$(CXX) -o $@ SemanticWikipediaExtractorMain.cpp SemanticWikipediaExtractor.o $(GLOBAL_OBJECTS) -lexpat # $(BOOST_REGEX_LIB)

		
SemanticWikipediaExtractor.o: SemanticWikipediaExtractor.cpp $(HEADERS)
	$(CXX) -I $(BOOSTDIR) -c $<
		