include ../Makefile

OBJECTS = StringConverter.o WkSupport.o XmlToJson.o
HEADERS = StringConverter.h WkSupport.h TimerStatistics.h XmlToJson.h

BINARIES = TimerStatisticsMain XmlToJsonMain
TEST_BINARIES = $(basename $(wildcard *Test.cpp))
PERF_BINARIES = $(basename $(wildcard *Perf.cpp))
LIBS = libstringconverter.a

libstringconverter.a: StringConverter.o WkSupport.o
	rm -f $@
	ar cq $@ $^

StringConverterTest: StringConverterTest.cpp $(OBJECTS)
	$(CXX) -o $@ $^ -lgtest -lgtest_main -lpthread $(LIBS_INCLUDED)

StringConverterPerf: $(OBJECTS) StringConverterPerf.o
	$(CXX) -o $@ $^  $(LIBS_INCLUDED)

TimerStatisticsTest: TimerStatisticsTest.o TimerStatistics.o
	$(CXX) -o $@ $^ -lgtest -lgtest_main -lpthread $(LIBS_INCLUDED)

TimerStatisticsMain: TimerStatisticsMain.o TimerStatistics.o
	$(CXX) -o $@ $^

XmlToJsonMain: XmlToJsonMain.o XmlToJson.o
	$(CXX) -o $@ $^ $(LIBS_INCLUDED)

XmlToJsonTest: XmlToJsonTest.o XmlToJson.o
	$(CXX) -o $@ $^ -lgtest -lgtest_main -lpthread $(LIBS_INCLUDED)

%.o: %.cpp $(HEADERS)
	$(CXX) -c $<  $(LIBS_INCLUDED)
