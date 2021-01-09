CXX = g++
FLAGS = -O3 -Wall
STDLIB = /usr/lib32/libstdc++.so.5.0.7
PHOLIB = /home/hoffmaje/Work/Weitkaemper/phonetik/libtrapho.so
BINARIES = phonetik_main phonetikPerf

all: $(BINARIES)
	
phonetik_main: phonetik_main.cpp
	$(CXX) -o $@ $^ $(STDLIB) $(PHOLIB) -m32

phonetikPerf: phonetikPerf.cpp
	$(CXX) -o $@ $^ $(STDLIB) $(PHOLIB) -m32

clean:
	rm -f *.o $(BINARIES)
