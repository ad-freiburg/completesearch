include ../Makefile
-include $(STXXL_CONFIG)

# NEW 22Feb12 Ina:
# Added the possibilty to use the libstxxl-dev package.
# It's done by using a conditional, to provide backward-compatibility.
# Else there would be thrown an error, if libstxxl.so doesn't exit.
# /usr/include/include needs to be included, since libstxxl uses this directory
# to store the headerfiles, instead of /usr/include.
# I am not sure, why they did that. Probably it's just
# an error within the package.

# If libstxxl is available, use it instead of STXXL_CONFIG.
ifeq ($(wildcard /usr/lib/libstxxl.so), /usr/lib/libstxxl.so) 
  LIBS_INCLUDED += -I/usr/include/include -lstxxl
else
  CXX += $(STXXL_CPPFLAGS_STXXL)
  LIBS_INCLUDED += $(STXXL_LDLIBS)
endif

CXX += -fopenmp

HEADERS       = $(wildcard *.h)
OBJECTS       = BinarySort.o
BINARIES      = BinarySortMain ConvertBinaryToAscii ConvertAsciiToBinary
PERF_BINARIES =

ConvertBinaryToAscii: ConvertBinaryToAscii.cpp
	$(CXX) -o $@ $^ $(LIBS_INCLUDED)

ConvertAsciiToBinary: ConvertAsciiToBinary.cpp
	$(CXX) -o $@ $^ $(LIBS_INCLUDED)

BinarySortMain: BinarySortMain.cpp BinarySort.o
	$(CXX) $^ -o $@ $(LIBS_INCLUDED) -lpthread

%Test: %Test.o $(OBJECTS)
	$(CXX) -o $@ $^ -lgtest -lgtest_main -lpthread $(LIBS_INCLUDED)

%Perf: %Perf.o $(OBJECTS)
	$(CXX) -o $@ $^ $(LIBS_INCLUDED)

%.o: %.cpp $(HEADERS)
	$(CXX) -c $<
