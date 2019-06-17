include ../dblp/Makefile

ENCODING = utf8
LOCALE = en_US.utf8
ENABLE_FUZZY_SEARCH = 0 
ENABLE_SYNONYM_SEARCH = 0
ENABLE_BINARY_SORT = 0
ICE_CITE_MODE = 1
NORMALIZE_WORDS = 0
VERBOSITY = 1
SHOW_QUERY_RESULT = 0
SORT = sort -S 1G -T data/tmp
PARSER = ./dblp.parser
PARSER_OPTIONS = \
  --base-name=$(DB) \
  --encoding=$(ENCODING) \
  $(shell perl -e 'print $(ENABLE_SYNONYM_SEARCH) ? "--read-synonym-groups" : "";') \
  $(shell perl -e 'print $(NORMALIZE_WORDS) ? "--normalize-words" : "";') \
  $(shell perl -e 'print $(ICE_CITE_MODE) ? "--icecite-mode" : "";')

CS_CODE_DIR = codebase
DB = data/dblp+medline-test
PORT = 6201
PIDFILE = ~ 

startice:
	$(MAKE) $(DB).server-ice

%.server-ice: %.hybrid %.vocabulary %.docs.DB
	$(CS_CODE_DIR)/server/startCompletionServer \
	  --locale=$(LOCALE) \
          $(shell perl -e 'print $(ENABLE_FUZZY_SEARCH) ? "--enable-fuzzy-search" : "";') \
          $(shell perl -e 'print $(ENABLE_SYNONYM_SEARCH) ? "--enable-synonym-search" : "";') \
          $(shell perl -e 'print $(NORMALIZE_WORDS) ? "--normalize-words" : "";') \
          $(shell perl -e 'print $(FUZZY_NORMALIZE_WORDS) ? "--fuzzy-normalize-words" : "";') \
          --verbosity=$(VERBOSITY) \
          --log-file=$*.log \
	  -P $(PIDFILE)\
          --port=$(PORT) \
          $*.hybrid
	tail -f $*.log

pallice:
	$(MAKE) buildxml
	$(MAKE) pall

buildxml:
	echo "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" > $(BUILDXML_TARGET) ; \
	echo "<dblp>" >> $(BUILDXML_TARGET) ; \
	for file in $$(ls -rt $(BUILDXML_DIR)) ; do \
		cat $(BUILDXML_DIR)/$$file >> $(BUILDXML_TARGET) ; \
	done ; \
	echo "</dblp>" >> $(BUILDXML_TARGET)
	#cat $(BUILDXML_DIR)/* > $(BUILDXML_TARGET)
