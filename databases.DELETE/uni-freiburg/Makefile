include ../../codebase/Makefile

ENCODING = utf8
LOCALE = en_US.utf8
ENABLE_FUZZY_SEARCH = 1
ENABLE_SYNONYM_SEARCH = 1
ENABLE_BINARY_SORT = 0
NORMALIZE_WORDS = 1
SORT = sort -T data -S 1G
PARSER = ./uni-freiburg.parser
PARSER_OPTIONS = \
  --base-name=$(DB) \
  --encoding=$(ENCODING) \
  $(shell perl -e 'print $(ENABLE_SYNONYM_SEARCH) ? "--read-synonym-groups" : "";') \
  $(shell perl -e 'print $(NORMALIZE_WORDS) ? "--normalize-words" : "";')
DB = data/uni-freiburg
PORT = 12345

parser: uni-freiburg.parser.cpp
	$(MAKE) -C $(CS_CODE_DIR)/parser build
	$(CXX) -o uni-freiburg.parser -I../.. $^ -lexpat \
	  $(filter-out %Main.o %Test.o %Perf.o, $(wildcard $(CS_CODE_DIR)/parser/*.o)) \
	  $(filter-out %Main.o %Test.o %Perf.o, $(wildcard $(CS_CODE_DIR)/utility/*.o)) \
	  $(filter-out %Main.o %Test.o %Perf.o, $(wildcard $(CS_CODE_DIR)/synonymsearch/*.o))

