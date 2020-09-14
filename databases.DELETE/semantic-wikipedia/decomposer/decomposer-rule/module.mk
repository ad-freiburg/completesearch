MDIR = decomposer-rule
#SRC += $(filter-out %Main.cpp %Test.cpp, $(wildcard $(MDIR)/*.cpp))
SRC += decomposer-rule/RobustContextMarker.cpp
MAINFILES += $(filter %Main.cpp, $(wildcard $(MDIR)/*.cpp))
TESTFILES += $(filter %Test.cpp, $(wildcard $(MDIR)/*.cpp))
