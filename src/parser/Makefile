include ../Makefile

LIBS_INCLUDED += -lexpat -L $(CS_CODE_DIR)/synonymsearch -lsynonymsearch

HEADERS  = $(wildcard *.h)
OBJECTS  = CsvParser.o CsvParserOptions.o SimpleTextParser.o \
           StringConversion.o ../utility/StringConverter.o \
	   ../utility/WkSupport.o \
           UserDefinedIndexWords.o ParserBase.o XmlParserNew.o
BINARIES = CsvParserMain makeXml XmlParserNewExampleMain

.PRECIOUS: %.o

build:
	$(MAKE) $(BINARIES) $(TEST_BINARIES) $(PERF_BINARIES) $(LIBS)

%Main: %Main.o $(OBJECTS)
	$(CXX) -o $@ $^ $(LIBS_INCLUDED)

%Test: %Test.o $(OBJECTS)
	$(CXX) -o $@ $^ $(LIBS_INCLUDED) -lgtest -lgtest_main -lpthread

%Perf: %Perf.o $(OBJECTS)
	$(CXX) -o $@ $^ $(LIBS_INCLUDED)

# UserDefinedIndexWords.o: UserDefinedIndexWords.cpp UserDefinedIndexWords.h
# 	$(CXX) -c $< $(GTEST_DIR)/libgtest.a

%.o: %.cpp $(HEADERS)
	$(CXX) -o $@ -c $<

makeXml: makeXml.cpp
	$(CXX) -o $@ $<
