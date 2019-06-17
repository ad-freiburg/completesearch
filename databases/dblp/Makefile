include codebase/Makefile
include perf/Makefile

ifndef DBLP_DIR
  export DBLP_DIR    := $(patsubst %/,%,$(dir $(CURDIR)/$(firstword $(MAKEFILE_LIST))))
endif


#CXX_DEBUG = -g -pg

SHELL = /usr/local/bin/bash
ENCODING = utf8
LOCALE = en_US.utf8
ENABLE_FUZZY_SEARCH = 1 
FUZZY_SEARCH_ALGORITHM = simple
FUZZY_USE_BASELINE = 1
HYB_PREFIX_LENGTH = 3
ENABLE_SYNONYM_SEARCH = 0
ENABLE_BINARY_SORT = 1
PARSE_EXTENDED_DTD = 0
NORMALIZE_WORDS = 1
VERBOSITY = 1
#DOCUMENT_ROOT = "/home/dblp/completesearch/codebase/userinterface/jquery"
DISABLE_CDATA_TAGS = 1
SHOW_QUERY_RESULT = 0
USE_SUFFIX_FOR_EXACT_QUERY = 1
QUERY_TIMEOUT = 1500
CLEANUP_BEFORE_PROCESSING = 1
SORT = sort -T data -S 1G
PARSER = ./dblp.parser
#PARSER = $(CS_CODE_DIR)/parser/CsvParserMain
CSV = 0
MULTIPLE_TITLE = 1
WORD_SEPARATOR_FRONTEND = :
# WORD_SEPARATOR_BACKEND should not be quoted, since this results in problem
# when using regexes for generating the hybrid prefix list.
WORD_SEPARATOR_BACKEND  = !
INFO_DELIMITER = ''

PARSER_OPTIONS = \
  $(shell perl -e 'print $(CSV) ? "$(PARSER_OPTIONS_CSV)" : "";') \
  --base-name=$(DB) \
  --encoding=$(ENCODING) \
  --maps-directory=$(DBLP_DIR) \
  --word-part-separator-backend=${WORD_SEPARATOR_BACKEND} \
  $(shell perl -e 'print $(ENABLE_SYNONYM_SEARCH) ? "--read-synonym-groups" : "";') \
  $(shell perl -e 'print $(NORMALIZE_WORDS) ? "--normalize-words" : "";') \
  $(shell perl -e 'print $(PARSE_EXTENDED_DTD) ? "--parse-extended-dtd" : "";')
PARSER_OPTIONS_CSV = \
  --full-text=author,title,venue,year --show=dblp-record:xml\\;html-record:json \
  --filter=author,venue,year,title,authorname,firstauthor --facets=author,venue,year,authorname,type,firstauthor,na \
  --within-field-separator=\# --no-show-prefix=\"*\" \
  --allow-multiple-items=author,authorname,firstauthor \
  --field-format=dblp-record:text,html-record:text \

DB = data/dblp
PORT = 8181

start::
	@echo
	@echo "Warming cache for DBLP (after waiting 15 seconds) ..."
	@sleep 15 
	@curl -s "http://localhost:$(PORT)/?q=ct:author:*&h=0&c=3" | head -1
	@curl -s "http://localhost:$(PORT)/?q=ct:venue:*&h=0&c=3" | head -1
	@curl -s "http://localhost:$(PORT)/?q=ct:year:*&h=0&c=3" | head -1
	@curl -s "http://localhost:$(PORT)/?q=ct:type:*&h=0&c=3" | head -1
	@echo
	@$(MAKE) -s log-last-start | grep -v "^\["

log-last:
	@export L=tgrep -n "^\[ *1\]" $(DB).log | tail -1 | cut -d: -f1`; \
	export L=`expr $$L - 10`; \
	tail -f -n +$$L $(DB).log | grep "\[1m"

log-short:
	$(MAKE) log | grep "\[1m"

log-last-start:
	@export L=`grep -n "COMPLETION SERVER" $(DB).log | tail -1 | cut -d: -f1`; \
	sed -n "$$L,$$ p" < $(DB).log \
	  | grep -v "^\["

L = 100

log-php:
	tail -n $(L) -f /var/log/dblp/access.log | perl ip2host -ctos "dblp" | grep -v dblpclient

log-phpm:
	tail -n 5000 -f /var/log/dblp/access.log | perl ip2host -ctos "dblpmirror"

log-phpa:
	tail -n $(L) -f /var/log/dblp/access.log | perl ip2host -cts "dblp" | grep -v dblpclient

log-client:
	tail -n 100 -f /var/log/dblp/access.log | perl ip2host -ctos "dblpclient"

log-php-error: 
	        tail -f /var/log/dblp/error.log

log-apache-api:
	cat /var/log/apache2/dblp-access.log \
	  | \grep "search/api" \
	  | cut -c1-110 \
	  | perl -ne '$$i = s/^(\d+\.\d+\.\d+\.\d+)// ? $$1 : "123456789"; $$h = `host $$i`; chomp $$h; printf "%70s %s", $$h, $$_;'

clean_dblp:
	rm -f dblp.dtd
	rm -f dblp.xml.gz
	rm -f dblp.parser

# Make a local backup of the DBLP index files.
backup-dblp:
	cp -f data/dblp.hybrid backup/
	cp -f data/dblp.docs.DB backup/
	cp -f data/dblp.vocabulary backup/

# Copy DBLP-Test index files to DBLP index files
update-dblp-index:
	for F in data/dblp-test.*; do cp -f $$F $${F/dblp-test/dblp}; done
	#for F in data/dblp-test.hybrid data/dblp-test.docs.DB data/dblp-test.vocabulary data/dblp-test.fuzzysearch*;

download:
	rm -f dblp.xml.gz
	rm -f dblp.dtd
	wget -nv http://dblp.uni-trier.de/xml/dblp.xml.gz
	gunzip -f dblp.xml.gz
	mv -f dblp.xml data/dblp.xml.original
	perl -ne 's/<(\/?(sup|sub|i||tt))>/&lt;$$1&gt;/g;print;' data/dblp.xml.original > $(DB).xml
	wget -nv http://dblp.uni-trier.de/xml/dblp.dtd
	rm -f data/dblp.xml.original

download-sorted-xml:
	rm -f dblp-sorted.xml.gz
	wget -nv http://dblp2.uni-trier.de/db/indices/x-tree/sortedDBLP.xml.gz
	gunzip -f sortedDBLP.xml.gz
	mv -f sortedDBLP.xml data/dblp-sorted.xml.original
	perl -ne 's/<(\/?(sup|sub|i||tt))>/&lt;$$1&gt;/g;print;' data/dblp-sorted.xml.original > data/dblp-csv.xml
	rm -f data/dblp-sorted.xml.original


%.xmllint: %.xml
	xmllint --loaddtd --noout $*.xml

# TODO: does not do what Hannah wants it to do.
.PHONEY: build-stuff

build-stuff:
	$(MAKE) -C $(CS_CODE_DIR)/parser build;
	$(MAKE) -C $(CS_CODE_DIR)/utility build;

dblp.parser: dblp.parser.cpp build-stuff
	$(CXX) -o $@ $< \
	  $(filter-out %Main.o %Test.o %Perf.o, $(wildcard $(CS_CODE_DIR)/parser/*.o)) \
	  $(filter-out %Main.o %Test.o %Perf.o, $(wildcard $(CS_CODE_DIR)/utility/*.o)) \
	  $(filter-out %Main.o %Test.o %Perf.o, $(wildcard $(CS_CODE_DIR)/synonymsearch/*.o)) \
	  -lexpat

dblp.xml2csv: dblp.xml2csv.cpp HtmlEntityDecoder.cpp
	$(CXX) -o $@ $^ -lboost_regex-mt

pall-csv: 
	make CSV=1 PORT=8182 PARSER=$(CS_CODE_DIR)/parser/CsvParserMain pall

convert-xml-to-csv: dblp.xml2csv
	./dblp.xml2csv --within-field-separator=\# --no-show-facet-prefix=* --base-name=$(DB)
