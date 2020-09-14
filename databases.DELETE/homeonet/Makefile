include codebase/Makefile

ENCODING = utf8
LOCALE = en_US.utf8
ENABLE_FUZZY_SEARCH = 0
FUZZY_SEARCH_ALGORITHM = simple
FUZZY_USE_BASELINE = 1
HYB_PREFIX_LENGTH = 3
ENABLE_SYNONYM_SEARCH = 0
ENABLE_BINARY_SORT = 0
PARSE_EXTENDED_DTD = 1
NORMALIZE_WORDS = 1
VERBOSITY = 1
SHOW_QUERY_RESULT = 0
SORT = sort -T data -S 1G
PARSER=./homeonet.parser
DB = data/hz-liste-filtered
FILTERLIST = mailsToFilter.list
PORT=8234

PARSER_OPTIONS = \
	--base-name=$(DB) --encoding=$(ENCODING) \
	--maps-directory=$(CS_CODE_DIR)/utility \
	$(shell perl -e 'print $(ENABLE_SYNONYM_SEARCH) ? "--read-synonym-groups" : "";') \
  	$(shell perl -e 'print $(NORMALIZE_WORDS) ? "--normalize-words" : "";') \
  	$(shell perl -e 'print $(PARSE_EXTENDED_DTD) ? "--parse-extended-dtd" : "";')

PARSER_OPTIONS_CSV = \
	--full-text=author,title,excerpts --show=author,title,url \
	--filter=author --facets=author --excerpts=excerpts \
	--base-name=$(DB) --encoding=$(ENCODING) \
	--maps-directory=$(CS_CODE_DIR)/utility \
	--old-words-format \
	$(shell perl -e 'print $(ENABLE_SYNONYM_SEARCH) ? "--read-synonym-groups" : "";') \
  	$(shell perl -e 'print $(NORMALIZE_WORDS) ? "--normalize-words" : "";') \
  	$(shell perl -e 'print $(PARSE_EXTENDED_DTD) ? "--parse-extended-dtd" : "";')

homeonet.parser: homeonet.parser.cpp
	$(MAKE) -C $(CS_CODE_DIR)/parser build
	$(MAKE) -C $(CS_CODE_DIR)/utility build
	$(CXX) -o homeonet.parser $^ -lexpat \
	  $(filter-out %Main.o %Test.o %Perf.o, $(wildcard $(CS_CODE_DIR)/parser/*.o)) \
	  $(filter-out %Main.o %Test.o %Perf.o, $(wildcard $(CS_CODE_DIR)/utility/*.o)) \
	  $(filter-out %Main.o %Test.o %Perf.o, $(wildcard $(CS_CODE_DIR)/synonymsearch/*.o))

ppall-xml:
	rm -rf homeonet.parser
	$(MAKE) homeonet.parser
	python convertmbox.py XML $(DB)
	$(MAKE) pall PARSER=./homeonet.parser

ppall-csv:
	python convertmbox.py CSV $(DB)
	$(MAKE) pall PARSER=$(CS_CODE_DIR)/parser/CsvParserMain "PARSER_OPTIONS=$(PARSER_OPTIONS_CSV)"

filter:
	python filter-mbox.py data/hz-liste.mbox $(DB).mbox $(FILTERLIST)

download:
	curl http://lists.informatik.uni-freiburg.de/pipermail/homeo.mbox/homeo.mbox 2>/dev/null > data/hz-liste.mbox.13Dec11-today
	cat data/hz-liste.mbox.14Dec00-23Nov07 data/hz-liste.mbox.28Nov07-03Jan12 data/hz-liste.mbox.13Dec11-today > data/hz-liste.mbox
	@echo
	ls -hltr data/hz-liste.mbox*

%.xmllint: %.xml
	xmllint --nonet --loaddtd --noout $*.xml


