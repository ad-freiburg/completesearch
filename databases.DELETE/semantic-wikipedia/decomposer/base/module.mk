MDIR = base
SRC += $(filter-out %Main.cpp %Test.cpp, $(wildcard $(MDIR)/*.cpp))
MAINFILES += $(filter %Main.cpp, $(wildcard $(MDIR)/*.cpp))
TESTFILES += $(filter %Test.cpp, $(wildcard $(MDIR)/*.cpp))

