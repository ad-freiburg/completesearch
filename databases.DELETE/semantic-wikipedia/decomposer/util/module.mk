MDIR = util
SRC += util/ContextDecomposerUtil.cpp
#MAINFILES += $(filter %Main.cpp, $(wildcard $(MDIR)/*.cpp))
#TESTFILES += $(filter %Test.cpp, $(wildcard $(MDIR)/*.cpp))
TESTFILES += util/ContextDecomposerUtilTest.cpp