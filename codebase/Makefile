# Generic complete search index building makefile.
#
# USAGE INFO:
#
# 1. Each application should be in its own folder, with its own Makefile. The
# first line of that Makefile should include this Makefile (include <path to
# this Makefile>).
#
# 2. All files in that folder pertaining to the application should be prefixed
# with the same short name representing the application, for example "dblp". Not
# the Makefile, of course, that should simply be called Makefile. In the
# Makefile, right after the include set the DB variable to the application name,
# for example DB = dblp.
#
# 3. Each application needs to have its own parser. Our standard way is to
# subclass from our xmlparser. The parser source code is then in the file
# $(DB).parse.cpp. The parse target (make parse) will compile this code and run
# the parser, and produce the files $(DB).words_unsorted and
# $(DB).docs_unsorted.
#
# 4. The two other basic targets are "sort" and "index". So, assuming that there
# already is an xml file $(DB).xml, the standard pipeline can be run with "make
# parse sort index". The end product will be the three index files $(DB).hybrid,
# $(DB).docs.DB, and $(DB).vocabulary.

# The following software needs to be installed besides Completesearch:
#  * Google Test 1.6.0 (http://code.google.com/p/googletest/)
#  * Google hash map (http://code.google.com/p/google-sparsehash/)
#  * Expat XML parser (apt-get install libexpat1-dev)
#  * Zlib compression library (apt-get install zlib1g-dev)
#  * Stxxl external sorting library (http://stxxl.sourceforge.net/)

# Do not delete any intermediate files.
.PRECIOUS: %.docs-sorted %.docs-unsorted %.words %.docs.db %.hybrid %.vocabulary


# SECTION 1: VARIABLES.

ifndef CS_CODE_DIR
  export CS_CODE_DIR    := $(patsubst %/,%,$(dir $(CURDIR)/$(lastword $(MAKEFILE_LIST))))
endif

# CS_CODE_DIR      = /home/$(USER)/completesearch/codebase
CS_BIN_DIR       = $(CS_CODE_DIR)/server
STXXL_SORT_DIR   = $(CS_CODE_DIR)/binarysort
STXXL_CONFIG     = /opt/stxxl/stxxl.mk
LIBS_INCLUDED    = -L$(CS_CODE_DIR)/gtest
TEST_REPORTS_DIR = $(CS_CODE_DIR)/test-reports

PARSER         = $(CS_CODE_DIR)/parser/CsvParserMain
PARSER_OPTIONS =
XML_OPTS       =
SORT           = sort
LOCALE_POSIX   = export LC_ALL=POSIX
SHELL         := /bin/bash
PERL           = perl
PLATFORM       = $(shell uname -s)
CPPLINT        = $(CS_CODE_DIR)/utility/cpplint/cpplint.py

DB                    = collection_name
HYB_PREFIX_LENGTH     = 3
MAX_SIZE_HISTORY      = 512M
XML                   = $(DB).xml
LOCALE                = en_US.utf8
ENABLE_FUZZY_SEARCH   = 0
FUZZY_SEARCH_ALGORITHM = simple
ENABLE_SYNONYM_SEARCH = 0
ENABLE_BINARY_SORT    = 0
NORMALIZE_WORDS       = 0
FUZZY_NORMALIZE_WORDS = 0
FUZZY_COMPLETION_MATCHING = 1
USE_GENERALIZED_EDIT_DISTANCE_SLOW = 0
SCORE_AGGREGATIONS    = SSSS
VERBOSITY             = 1
SHOW_QUERY_RESULT     = 0
PORT                  = 8888
SERVER_OPTIONS_SHORT  =
WORD_SEPARATOR_FRONTEND = :
WORD_SEPARATOR_BACKEND  = !
QUERY_TIMEOUT         = 2000
DOCUMENT_ROOT 	      = ''
KEEP_IN_HISTORY_QUERIES  = 1
WARM_HISTORY_QUERIES     = 1
ENABLE_CORS           = 0
EXE_COMMAND 	      =

# SECTION 2: VARIABLES AND RULES FOR BUILDING CODE.

# Compiler and flags.
CXX_OPTIONS  = --std=c++11 -Wall# -Wno-uninitialized -Wno-deprecated -Wno-unused-function -fexceptions
# CXX_OPTIONS  = -Wall -Wno-uninitialized -Wno-deprecated -Wno-unused-function -fexceptions --std=gnu++0x
CXX_DEFINES  = -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_REENTRANT -DSTL_VECTOR -DDEBUG_PTHREAD_CREATE_TIME
CXX_DEBUG    = -g # -O3# -g #-DNDEBUG
CXX_INCLUDES = -I$(CS_CODE_DIR) -I$(CS_CODE_DIR)/gtest/include
# NOTE(Hannah, 26-01-2017): gtest uses tuple from different dir than sparsehash.
CXX_FIXES = -DGTEST_HAS_TR1_TUPLE=0 -DGTEST_USE_OWN_TR1_TUPLE=0
CXX = g++ $(CXX_OPTIONS) $(CXX_DEBUG) $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FIXES)

# All test and perf files (will be expanded only when used, so can be used in
# sub-directory).
TEST_BINARIES = $(basename $(wildcard *Test.cpp))
PERF_BINARIES = $(basename $(wildcard *Perf.cpp))

# Build code in every sub-directory.
build-all:
	echo $(CS_CODE_DIR)
	rm -f $(CS_CODE_DIR)/server/StartCompletionServer.o
	$(MAKE) -C $(CS_CODE_DIR)/gtest build
	$(MAKE) -C $(CS_CODE_DIR)/synonymsearch build
	$(MAKE) -C $(CS_CODE_DIR)/utility build
	$(MAKE) -C $(CS_CODE_DIR)/fuzzysearch build
	$(MAKE) -C $(CS_CODE_DIR)/binarysort build
	$(MAKE) -C $(CS_CODE_DIR)/parser build
	$(MAKE) -C $(CS_CODE_DIR)/server build

# Test code in every sub-directory.
test-all:
	$(MAKE) -C $(CS_CODE_DIR)/binarysort test
	$(MAKE) -C $(CS_CODE_DIR)/synonymsearch test
	$(MAKE) -C $(CS_CODE_DIR)/utility test
	$(MAKE) -C $(CS_CODE_DIR)/parser test
	$(MAKE) -C $(CS_CODE_DIR)/server test

# Remove all build products (object files, binaries, etc.) in all
# sub-directories.
clean-all:
	$(MAKE) -C $(CS_CODE_DIR)/gtest clean
	$(MAKE) -C $(CS_CODE_DIR)/parser clean
	$(MAKE) -C $(CS_CODE_DIR)/binarysort clean
	$(MAKE) -C $(CS_CODE_DIR)/fuzzysearch clean
	$(MAKE) -C $(CS_CODE_DIR)/synonymsearch clean
	$(MAKE) -C $(CS_CODE_DIR)/utility clean
	$(MAKE) -C $(CS_CODE_DIR)/server clean
	rm -f $(TEST_REPORTS_DIR)/*.xml

# Generic build target that should work in every sub-directory.
build:
	$(MAKE) $(BINARIES) $(TEST_BINARIES) $(PERF_BINARIES) $(LIBS)

# Generic checkstyle target that should work in every sub-directory.
checkstyle:
	python $(CPPLINT) *.h *.cpp

# Generic test target that should work in every sub-directory.
test:
	mkdir -p $(TEST_REPORTS_DIR)
	for T in *Test *Test.exe; \
	  do [[ $${T:0:1} == "*" ]] && continue; \
	  export LD_LIBRARY_PATH=$(GTEST_LIBS); ./$$T --gtest_output=xml; \
	  TESTNAME="`basename $${T} .exe`"; \
	  mv test_detail.xml $${TESTNAME}.xml; \
	  $(PERL) $(CS_CODE_DIR)/utility/gtest-hudson.pl \
	    $${TESTNAME}.xml $(TEST_REPORTS_DIR); \
		rm -f $${TESTNAME}.xml; done

# Generic perf target that should work in every sub-directory.
perf:
	for P in *Perf; \
	  do ./$$P; done

# Generic clean target that should work in every sub-directory.
clean:
	rm -f *.o
	rm -f *Main.exe *Test.exe *Perf.exe
	rm -f $(BINARIES) $(TEST_BINARIES) $(PERF_BINARIES) $(LIBS)

# Clean the results from the precomputation (make pall), EXCEPT the final
# products (.hybrid, .docs.DB, .vocabulary) and the log (.log). Show which
# files of the form $(DB).* are left afterwards.
pclean:
	@rm -f $(DB).words-*
	@rm -f $(DB).docs-*
	@rm -f $(DB).make*
	@rm -f $(DB).parse-log
	@rm -f $(DB).keys*
	@rm -f $(DB).hybrid.build-*
	@rm -f $(DB).hybrid.from-*
	# Older files which are not produced anymore
	@rm -f $(DB).cluster*
	@rm -f $(DB).permuted*
	ls -lhtr $(DB).*

# Like pclean, but also removes the final products and the log.
pclean-all: pclean
	@rm -f $(DB).hybrid
	@rm -f $(DB).hybrid.*
	@rm -f $(DB).docs.DB
	@rm -f $(DB).vocabulary
	@rm -f $(DB).vocabulary?*
	@rm -f $(DB).log
	@rm -f $(DB).fuzzysearch*
	@rm -f $(DB).keep-in-history

# HACK(bast): Target to handle the current problem that code is not relinked
# when one of the libraries like fuzzysearch changes.
relink-all:
	$(MAKE) -C $(CS_CODE_DIR)/parser relink
	$(MAKE) -C $(CS_CODE_DIR)/binarysort relink
	$(MAKE) -C $(CS_CODE_DIR)/fuzzysearch relink
	$(MAKE) -C $(CS_CODE_DIR)/synonymsearch relink
	$(MAKE) -C $(CS_CODE_DIR)/utility relink
	$(MAKE) -C $(CS_CODE_DIR)/server relink

relink:
	for B in $(BINARIES) StartCompletionServer; do rm -f $$B.o; done
	$(MAKE) build

COVERAGE_NAME="coverage.xml"
check-all:
	$(MAKE) "CXX_DEBUG=-O0 -g -fprofile-arcs -ftest-coverage -fPIC -lgcov" clean-all build-all test-all
	$(MAKE) -C $(CS_CODE_DIR)/example/xml/ test-pall-xml
	$(MAKE) -C $(CS_CODE_DIR)/example/csv/ test-pall-csv
	$(CS_CODE_DIR)/utility/gcovr -x -d -r . -e $(CS_CODE_DIR)/gtest/ -e $(CS_CODE_DIR)/test-reports/ -e '.*Test' > $(COVERAGE_NAME)
	# Clean cobertura output "dir...." should be erased, same for "dir/../",
	# "./" and ".." at the end of the line
	sed -i 's/\w*\.\.\.\.//g' $(COVERAGE_NAME)
	sed -i 's/\w*\/\.\.\/*//g' $(COVERAGE_NAME)
	sed -i 's/\.\///g' $(COVERAGE_NAME)
	sed -i 's/\.\.\">$$/\">/g' $(COVERAGE_NAME)
	python $(CPPLINT) */*.cpp */*.h &> cpplint.xml || echo 0


# SECTION 3 : RULES FOR RUNNING THE COMPLETESEARCH PIPELINE.

xml:
	$(PERL) $(DB).xml.make.pl $(XML_OPTS) dblp.xml ARCHIVE > $(DB).xml

parse-xml:
	$(MAKE) $(DB).parser
	./$(DB).parser $(PARSE_OPTS) $(XML) $(DB).docs-unsorted $(DB).words-unsorted.ascii

parse:
	export PARSER_OPTIONS_ADDITIONS=""; \
	if [ "$(ENABLE_FUZZY_SEARCH)" -eq "1" ]; then \
	$(PARSER) $(PARSER_OPTIONS) \
	  --write-vocabulary --output-word-frequencies \
  	  --word-part-separator-backend=${WORD_SEPARATOR_BACKEND}; \
	  $(MAKE) fuzzysearch; \
	  export PARSER_OPTIONS_ADDITIONS="--read-fuzzy-search-clusters"; fi; \
	if [ "$(ENABLE_BINARY_SORT)" -eq "0" ]; then \
	  $(PARSER) $(PARSER_OPTIONS) $$PARSER_OPTIONS_ADDITIONS \
	    --write-vocabulary --write-docs-file --write-words-file-ascii \
  	    --word-part-separator-backend=${WORD_SEPARATOR_BACKEND}; \
	else \
	  $(PARSER) $(PARSER_OPTIONS) $$PARSER_OPTIONS_ADDITIONS \
	    --write-vocabulary \
  	    --word-part-separator-backend=${WORD_SEPARATOR_BACKEND}; \
	  $(PARSER) $(PARSER_OPTIONS) $$PARSER_OPTIONS_ADDITIONS \
	    --read-vocabulary --write-docs-file --write-words-file-binary \
  	    --word-part-separator-backend=${WORD_SEPARATOR_BACKEND}; fi

sort:
	if [ "$(ENABLE_BINARY_SORT)" -eq "0" ]; \
	  then $(MAKE) $(DB).words-sorted.ascii; \
	       $(MAKE) vocabulary; \
	  else $(MAKE) $(DB).words-sorted.binary; fi
	$(MAKE) $(DB).docs-sorted

vocabulary:
	$(LOCALE_POSIX); cut -f1 $(DB).words-sorted.ascii | $(SORT) -u > $(DB).vocabulary

# Build fuzzy search data structure + clustering. MARJAN'S NEW METHOD.
fuzzysearch:
	# Remove all : words from the vocabulary (but save it, so that we have
	# the original one in the end, including words like :filter:... which
	# are needed when writing the output in binary).
	# cp $(DB).vocabulary $(DB).vocabulary.BACKUP
	$(LOCALE_POSIX); $(SORT) -c $(DB).vocabulary
	cat $(DB).vocabulary | $(PERL) -ne 'print unless /$(WORD_SEPARATOR_BACKEND)/;' > $(DB).vocabulary.TMP
	mv -f $(DB).vocabulary.TMP $(DB).vocabulary
	ls -alh $(DB).vocabulary
	@echo
	# Same for the new .vocabulary+frequencies.
	$(LOCALE_POSIX); cut -f2 $(DB).vocabulary+frequencies | sort -c -n
	cat $(DB).vocabulary+frequencies | $(PERL) -ne 'print unless /$(WORD_SEPARATOR_BACKEND)/;' > $(DB).vocabulary+frequencies.TMP
	mv -f $(DB).vocabulary+frequencies.TMP $(DB).vocabulary+frequencies
	ls -alh $(DB).vocabulary+frequencies
	@echo
	# Clustering.
	printf "\n\n*** BUILD CLUSTERING ***\n";
	$(CS_CODE_DIR)/fuzzysearch/buildFuzzySearchClusters \
	  -a 1 -s 1 -i 100 -o 1 -m 100 -q 100 \
	  $(shell $(PERL) -e 'print $(FUZZY_COMPLETION_MATCHING) ? "-c" : "";') \
	  --locale=$(LOCALE) \
	  $(shell $(PERL) -e 'print $(FUZZY_NORMALIZE_WORDS) ? "--normalize" : "";') \
	  $(shell $(PERL) -e 'print $(FUZZY_SEARCH_ALGORITHM) eq "simple" ? "-T" : "";') \
	  $(DB)

enhance-words:
	$(LOCALE_POSIX); $(SORT) -k1,1 -k2,2n -k4,4n $(DB).words.enhancing.* \
	  | $(SORT) -m -k1,1 -k2,2n -k4,4n $(DB).words - \
	  > $(DB).words.enhanced
	mv -f $(DB).words.enhanced $(DB).words

enhance-docs:
	$(MAKE) -C bin enhanceExcerpts
	$(PERL) -ne 's/$(WORD_SEPARATOR_BACKEND)$(WORD_SEPARATOR_BACKEND)[^\t]+//; print;' < $(DB).words.enhancing.spell > $(DB).words.enhancing.spell.tmp
	$(CS_DIR)/enhanceExcerpts $(DB).docs $(DB).docs.enhanced $(DB).words.enhancing.spell.tmp
	rm -f $(DB).words.enhancing.spell.tmp
	$(LOCALE_POSIX); $(SORT) -c -k1,1n $(DB).docs
	mv -f $(DB).docs.enhanced $(DB).docs

index:
	if [ "$(ENABLE_BINARY_SORT)" -eq "0" ]; \
	  then $(MAKE) $(DB).hybrid.from-ascii; \
	  else $(MAKE) $(DB).hybrid.from-binary; fi
	$(MAKE) $(DB).docs.DB

start::
	$(MAKE) $(DB).start

stop::
	$(CS_BIN_DIR)/startCompletionServer --kill $(PORT)

log:
	tail -n 200 -f $(DB).log

pall:
	@echo "BEGIN \"make pall\" : " `date` > $(DB).make-begin
	# $(MAKE) parse-xml
	$(MAKE) parse
	$(MAKE) sort
	$(MAKE) index
	@echo "END   \"make pall\" : " `date` > $(DB).make-end
	@echo; ls -lhtr $(DB).*
	@echo; cat $(DB).make-begin $(DB).make-end
	@echo

pall+spell:
	#$(MAKE) xml
	#$(MAKE) parse PARSE_OPTS="-i $(PARSE_OPTS)"
	$(MAKE) parse
	$(MAKE) sort
	$(MAKE) phrases
	$(MAKE) spell
	$(MAKE) enhance-words
	#$(MAKE) enhance-docs
	$(MAKE) index

pack:
	tar -cf $(DB).tar $(DB).hybrid $(DB).docs.DB $(DB).vocabulary
	bzip2 $(DB).tar

# make vocabulary with frequency (for SIGIR'08 docs.DB)
freqranks:
	$(LOCALE_POSIX); cut -f1 $(DB).words \
	  | uniq -c \
	  | $(SORT) -k1,1nr \
	  | $(PERL) -e 'while(<>){++$$c; /^\s*\d+\s*(\S+)\s*$$/; print "$$1\t$$c\n";}' \
	  > $(DB).vocabulary+freqranks
	tail $(DB).vocabulary+freqranks
	wc -l $(DB).vocabulary+freqranks

%.parser: %.parser.cpp $(CS_CODE_DIR)/parser/XmlParser.cpp $(CS_CODE_DIR)/synonymsearch/libsynonymsearch.a
	g++ -O6 -I $(CS_CODE_DIR) -o $@ $^ -lexpat

%.start: %.hybrid %.vocabulary %.docs.DB
	$(CS_BIN_DIR)/startCompletionServer \
		$(SERVER_OPTIONS_SHORT) \
	  --locale=$(LOCALE) \
          --max-size-history=$(MAX_SIZE_HISTORY) \
	  $(shell $(PERL) -e 'print $(ENABLE_FUZZY_SEARCH) ? "--enable-fuzzy-search" : "";') \
	  $(shell $(PERL) -e 'print $(ENABLE_SYNONYM_SEARCH) ? "--enable-synonym-search" : "";') \
	  $(shell $(PERL) -e 'print $(NORMALIZE_WORDS) ? "--normalize-words" : "";') \
	  $(shell $(PERL) -e 'print $(DISABLE_CDATA_TAGS) ? "--disable-cdata-tags" : "";') \
	  $(shell $(PERL) -e 'print $(FUZZY_NORMALIZE_WORDS) ? "--fuzzy-normalize-words" : "";') \
	  $(shell $(PERL) -e 'print $(SHOW_QUERY_RESULT) ? "--show-query-result" : "";') \
	  $(shell $(PERL) -e 'print $(USE_SUFFIX_FOR_EXACT_QUERY) ? "--use-suffix-for-exact-query" : "";') \
	  $(shell $(PERL) -e 'print $(CLEANUP_BEFORE_PROCESSING) ? "--cleanup-query-before-processing" : "";') \
	  $(shell $(PERL) -e 'print $(MULTIPLE_TITLE) ? "--info-delimiter=$(INFO_DELIMITER)" : "";') \
	  $(shell $(PERL) -e 'print $(USE_GENERALIZED_EDIT_DISTANCE_SLOW) ? "--use-generalized-edit-distance-slow" : "";') \
	  $(shell $(PERL) -e 'print $(KEEP_IN_HISTORY_QUERIES) ? "--keep-in-history-queries $(DB).keep-in-history" : "";') \
	  $(shell $(PERL) -e 'print $(WARM_HISTORY_QUERIES) ? "--warm-history-queries $(DB).keep-in-history" : "";') \
	  $(shell $(PERL) -e 'print $(ENABLE_CORS) ? "--enable-cors" : "";') \
	  --score-aggregations=$(SCORE_AGGREGATIONS) \
	  --verbosity=$(VERBOSITY) \
	  --document-root=$(DOCUMENT_ROOT) \
	  --exe-command=$(EXE_COMMAND) \
	  --query-timeout=$(QUERY_TIMEOUT) \
	  --word-part-separator-backend=$(WORD_SEPARATOR_BACKEND) \
	  --word-part-separator-frontend=$(WORD_SEPARATOR_FRONTEND) \
	  --log-file=$*.log \
	  --port=$(PORT) \
	  $*.hybrid
# $(shell $(PERL) -e 'print $(FUZZY_USE_BASELINE) ? "--use-baseline-fuzzysearch" : "";') \

# Extract certain kinds of prefixes (explained for each of the lines below),
# and sort -u them.  This yields the block boundaries for the HBY index. The
# block boundaries are key for a high performance. In general:
#
# 1. Blocks must not be too small: a completion query for prefix* is efficient
# only if all matching words are in one block. If they are spread over several
# blocks, these blocks have to be merged, which is inherently expensive.
#
# 2. Blocks must not be too large: the query engine always reads whole blocks,
# so if the containing block is too large (e.g. pre*), it is also slow.
#
# In the follow comments, HYB_PREFIX_LENGTH = 3 and WORD_SEPARATOR_BACKEND = :
# and ? is a word character that is not WORD_SEPARATOR_BACKEND. This makes the
# comments easier to read and understand.
%.hybrid.prefixes: S = $(WORD_SEPARATOR_BACKEND)
%.hybrid.prefixes: L = $(HYB_PREFIX_LENGTH)
%.hybrid.prefixes: %.vocabulary
	# Extract all prefixes of the form ???
	${LOCALE_POSIX}; grep -v "${S}" $*.vocabulary | cut -c1-${L} \
	        | sort -u > $*.hybrid.prefixes_1
	# Extract all prefixes of the form ct:<word>: (old format, but doesn't harm)
	${LOCALE_POSIX}; nice perl -ne '/^(ct${S}[^${S}]+${S})/ and print "$$1\n";' < $*.vocabulary \
	        | sort -u > $*.hybrid.prefixes_2
	${LOCALE_POSIX}; perl -ne 's/$$/*/; print;' < $*.hybrid.prefixes_2 > $*.keep-in-history
	# Extract all prefixes of the form ce:<word>:??? (old format, but doesn't harm)
	${LOCALE_POSIX}; perl -ne '/^(ce${S}[^${S}]+${S}[^${S}]{${L}})/ and print "$$1\n";' < $*.vocabulary \
	        | sort -u > $*.hybrid.prefixes_3
	# Extract all prefixes of the form cn:<word>: (old format, but doesn't harm)
	${LOCALE_POSIX}; perl -ne '/^(cn${S}[^${S}]{${L}})/ and print "$$1\n";' < $*.vocabulary \
	        | sort -u > $*.hybrid.prefixes_4
	# Extract all prefixes of the form C:<word>: (fuzzy search)
	${LOCALE_POSIX}; perl -ne '/^(C${S}[^${S}]+${S})/ and print "$$1\n";' < $*.vocabulary \
	        | sort -u > $*.hybrid.prefixes_5
	# Extract all prefixes of the form :filter:<word>:??? (filter search, new format)
	${LOCALE_POSIX}; perl -ne '/^(${S}filter${S}[^${S}]+${S}[^C${S}\n]{${L}})/ and print "$$1\n";' < $*.vocabulary \
	        | sort -u > $*.hybrid.prefixes_6
	# Extract all prefixes of the form :filter:<word>:C:<word>: (fuzzy filter search))
	${LOCALE_POSIX}; perl -ne '/^(${S}filter${S}[^${S}]+${S}C${S}[^${S}]+${S})/ and print "$$1\n";' < $*.vocabulary \
	        | sort -u > $*.hybrid.prefixes_7
	# Extract all prefixes of the form :facet:<word>: (new format) and write to $(DB).keep-in-history
	${LOCALE_POSIX}; perl -ne '/^(${S}facet${S}[^${S}]+${S})/ and print "$$1\n";' < $*.vocabulary \
	        | sort -u > $*.hybrid.prefixes_8
	${LOCALE_POSIX}; perl -ne 's/$$/*/; print;' < $*.hybrid.prefixes_8 >> $*.keep-in-history
	# Extract all prefixes of the form :facetid:<word>:??? (new format)
	${LOCALE_POSIX}; perl -ne '/^(${S}facetid${S}[^${S}]+${S}[^C${S}\n]{${L}})/ and print "$$1\n";' < $*.vocabulary \
	        | sort -u > $*.hybrid.prefixes_9
	${LOCALE_POSIX}; sort -m $*.hybrid.prefixes_* \
	        | grep -v "^#" \
	        | perl -ne 'chomp; print "$$_\n" unless /[\x00-\x20\x7f-\xff]/;' \
	        > $*.hybrid.prefixes
	rm -f $*.hybrid.prefixes_*

# TODO(bast): throw out only thoses prefixes, where the *last* character is a
# non-printable one. Right now all prefixes are thrown out which contain such a
# character anywhere. Like that all words starting with Umlauts for example, end
# up together in one block, which is inefficient.
%.hybrid.from-binary: %.words-sorted.binary
	rm -f $*.hybrid.prefixes
	$(MAKE) $*.hybrid.prefixes
	$(CS_BIN_DIR)/buildIndex -b $*.hybrid.prefixes -f BINARY HYB $*.ANY_SUFFIX_WITHOUT_DOT
	  2> $*.hybrid.build-index-errors | tee $*.hybrid.build-index-log
	ln -sf $*.hybrid $@

# Build a hybrid index from an ASCII words file.
# TODO(Hannah): Potential problem in $(DB).hybrid.prefixes_2: can contain ' or
# -, both of which crash buildIndex, don't know why.
%.hybrid.from-ascii: %.words-sorted.ascii
	rm -f $*.hybrid.prefixes
	$(MAKE) $*.hybrid.prefixes
	$(CS_BIN_DIR)/buildIndex -b $*.hybrid.prefixes -f ASCII HYB $*.ANY_SUFFIX_WITHOUT_DOT
	  2> $*.hybrid.build-index-errors | tee $*.hybrid.build-index-log
	ln -sf $*.hybrid $@

%.docs.DB: %.docs-sorted
	$(CS_BIN_DIR)/buildDocsDB -f $^
	mv -f $*.docs-sorted.DB $*.docs.DB

%.docs.db: %.docs-sorted
	rm -f $*.docs.db
	$(TOOLDIR)/ExcerptsToDB $*.docs $*.docs.db

%.docs-sorted: %.docs-unsorted
	$(LOCALE_POSIX); $(SORT) -b -k1,1n $*.docs-unsorted > $*.docs-sorted

%.words-sorted.ascii: %.words-unsorted.ascii
	@$(LOCALE_POSIX); \
	$(SORT) -b -k1,1 -k2,2n -k4,4n $< > $@

%.words-sorted.binary: %.words-unsorted.binary
	cp -f $< $@
	$(CS_CODE_DIR)/binarysort/BinarySortMain \
	  --stxxl-path=$(shell pwd) \
	  --stxxl-disk-file=$(dir $(DB))stxxl.disk \
	  --binary-words-file=$@
