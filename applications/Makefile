# Makefile for building an index and starting a server with CompleteSearch
#
# NOTE: This includes $(DB)/Makefile below and will fail if not found.
#
# See README.md for explanations and the subdirectories in "applications" for
# example applications
#
# TODO: Paste generic docker command line here.

# Do not delete intermediate files created during index building (useful for
# debugging).
.PRECIOUS: %.docs-sorted %.docs-unsorted %.words

# SECTION 1: VARIABLES (includes $(DB)/Makefile below)

# Most of these variables are used as argument for startCompletionServer in the
# start: target below. See src/README.md for what they stand for.

show-config:
	@echo
	@echo "TEST (printing current directory and values of basic vars)"
	@echo
	@printf "pwd = " && pwd
	@echo "DATA_DIR = $(DATA_DIR)"
	@echo "DB = $(DB)"
	@echo "DB_PREFIX = $(DB_PREFIX)"
	@echo "CS_CODE_DIR = $(CS_CODE_DIR)"
	@echo "CS_BIN_DIR = $(CS_CODE_DIR)"
	@echo
	@echo "For the precomputation : make DATA_DIR=... DB=... pall"
	@echo "To start the server    : make DATA_DIR=... DB=... start"
	@echo "Precomputation + start : make DATA_DIR=... DB=... pall start"
	@echo


# Always set these by calling make DATA_DIR=<...> DB=<...> <target>. DATA_DIR is
# the directory of the index files. DB is the prefix of all index files and the
# name of the subdirectories for the data and the config.
DB             = set_db_name_in_Makefile
DATA_DIR       = set_data_dir_in_Makefile

# Directory with the code and the compiled binaries.
CS_CODE_DIR    = /src
CS_BIN_DIR     = $(CS_CODE_DIR)/server
CS_PW_DIR      = $(CS_CODE_DIR)/partialwords
MAPS_DIR       = $(CS_CODE_DIR)/utility
PARSER         = $(CS_CODE_DIR)/parser/CsvParserMain

DB_PREFIX      = $(DATA_DIR)/$(DB)
PARSER_OPTIONS =
SORT           = sort
LOCALE_POSIX   = export LC_ALL=POSIX
SHELL         := /bin/bash
PERL           = perl
JOIN           = /usr/bin/join

HYB_PREFIX_LENGTH     = 3
MAX_SIZE_HISTORY      = 512M
LOCALE                = en_US.utf8
ENABLE_FUZZY_SEARCH   = 0
FUZZY_SEARCH_ALGORITHM = simple
ENABLE_SYNONYM_SEARCH = 0
ENABLE_BINARY_SORT    = 0
NORMALIZE_WORDS       = 0
FUZZY_NORMALIZE_WORDS = 0
FUZZY_COMPLETION_MATCHING = 1
USE_GENERALIZED_EDIT_DISTANCE_SLOW = 0
PARTIAL_WORDS         = 0
PW_ADDITIONAL_VOCAB   = $(CS_PW_DIR)/partialwords/american-english.vocabulary
PW_MIN_WORD_LENGTH    = 5
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
PID_FILE_FORMAT       = $(DB_PREFIX).completesearch_%s_%s.pid

include $(DB)/Makefile

# SECTION 2: RULES FOR RUNNING THE COMPLETESEARCH PIPELINE.

# TODO: This works, but it's ugly, do we really need a huge nested if-else here?
parse:
	@export PARSER_OPTIONS_ADDITIONS=""; \
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
	  then $(MAKE) $(DB_PREFIX).words-sorted.ascii; \
	       $(MAKE) vocabulary; \
	  else $(MAKE) $(DB_PREFIX).words-sorted.binary; fi
	$(MAKE) $(DB_PREFIX).docs-sorted

partialwords:
	mv $(DB_PREFIX).words-unsorted.ascii $(DB_PREFIX).words-unsorted.ascii.TMP
	mv $(DB_PREFIX).words-sorted.ascii $(DB_PREFIX).words-sorted.ascii.TMP
	$(CS_PW_DIR)/PartialWordsMain $(DB_PREFIX).vocabulary $(PW_ADDITIONAL_VOCAB) $(PW_MIN_WORD_LENGTH) \
	  | grep -v "^:" > $(DB_PREFIX).partial-words && wc -l $(DB_PREFIX).partial-words | numfmt --grouping
	$(JOIN)	-t $$'\t' -j 1 $(DB_PREFIX).words-sorted.ascii.TMP $(DB_PREFIX).partial-words -o "2.2 1.2 1.3 1.4" \
	  | cat - $(DB_PREFIX).words-sorted.ascii.TMP > $(DB_PREFIX).words-unsorted.ascii
	$(MAKE) $(DB_PREFIX).words-sorted.ascii

vocabulary:
	$(LOCALE_POSIX); cut -f1 $(DB_PREFIX).words-sorted.ascii | $(SORT) -u > $(DB_PREFIX).vocabulary

index:
	if [ "$(ENABLE_BINARY_SORT)" -eq "0" ]; \
	  then $(MAKE) $(DB_PREFIX).hybrid.from-ascii; \
	  else $(MAKE) $(DB_PREFIX).hybrid.from-binary; fi
	$(MAKE) $(DB_PREFIX).docs.DB

start::
	$(MAKE) $(DB_PREFIX).start

stop::
	$(CS_BIN_DIR)/startCompletionServer --pid-file=$(PID_FILE_FORMAT) \
	  --kill $(PORT)

log:
	tail -n 200 -f $(DB_PREFIX).log

# Run the complete precomputation (parse, sort, index) with one target. Show all
# the produced files in the end and how much time it took.
pall::
	@echo "BEGIN \"make pall\" : " `date` > $(DB_PREFIX).make-begin
	$(MAKE) parse
	$(MAKE) sort
	if [ "$(PARTIAL_WORDS)" -eq "1" ]; then $(MAKE) partialwords; fi
	$(MAKE) index
	@echo "END   \"make pall\" : " `date` > $(DB_PREFIX).make-end
	@echo; echo "Files in directory $(DATA_DIR) with prefix $(DB):"
	@echo; cd $(DATA_DIR) && ls -lhtr $(DB).* && cd - > /dev/null
	@echo; cat $(DB_PREFIX).make-begin $(DB_PREFIX).make-end
	@echo

# Clean the results from the precomputation (make pall), EXCEPT the final
# products (.hybrid, .docs.DB, .vocabulary) and the log (.log). Show which
# files of the form $(DB_PREFIX). are left afterwards.
pclean:
	@rm -f $(DB_PREFIX).words-*
	@rm -f $(DB_PREFIX).docs-*
	@rm -f $(DB_PREFIX).make*
	@rm -f $(DB_PREFIX).parse-log
	@rm -f $(DB_PREFIX).keys*
	@rm -f $(DB_PREFIX).hybrid.build-*
	@rm -f $(DB_PREFIX).hybrid.from-*
	@rm -f $(DB_PREFIX).stxxl.disk
	# Older files which are not produced anymore
	@rm -f $(DB_PREFIX).cluster*
	@rm -f $(DB_PREFIX).permuted*
	ls -lhtr $(DB_PREFIX).*

# Like pclean, but also removes the final products and the log.
pclean-all: pclean
	@rm -f $(DB_PREFIX).hybrid
	@rm -f $(DB_PREFIX).hybrid.*
	@rm -f $(DB_PREFIX).docs.DB
	@rm -f $(DB_PREFIX).vocabulary
	@rm -f $(DB_PREFIX).vocabulary?*
	@rm -f $(DB_PREFIX).log
	@rm -f $(DB_PREFIX).fuzzysearch*
	@rm -f $(DB_PREFIX).keep-in-history



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
	  $(shell $(PERL) -e 'print $(KEEP_IN_HISTORY_QUERIES) ? "--keep-in-history-queries $(DB_PREFIX).keep-in-history" : "";') \
	  $(shell $(PERL) -e 'print $(WARM_HISTORY_QUERIES) ? "--warm-history-queries $(DB_PREFIX).keep-in-history" : "";') \
	  $(shell $(PERL) -e 'print $(ENABLE_CORS) ? "--enable-cors" : "";') \
	  --score-aggregations=$(SCORE_AGGREGATIONS) \
	  --verbosity=$(VERBOSITY) \
	  --maps-directory=$(MAPS_DIR) \
	  --document-root=$(DOCUMENT_ROOT) \
	  --exe-command=$(EXE_COMMAND) \
	  --query-timeout=$(QUERY_TIMEOUT) \
	  --word-part-separator-backend=$(WORD_SEPARATOR_BACKEND) \
	  --word-part-separator-frontend=$(WORD_SEPARATOR_FRONTEND) \
	  --pid-file=$(PID_FILE_FORMAT) \
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
	# Extract all prefixes of the form :facet:<word>: (new format) and write to $(DB_PREFIX).keep-in-history
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

# Build a hybrid (HYB) index from a BINARY words file.
#
# TODO(Hannah): throw out only thoses prefixes, where the *last* character is a
# non-printable one. Right now all prefixes are thrown out which contain such a
# character anywhere. Like that all words starting with Umlauts for example, end
# up together in one block, which is inefficient.
%.hybrid.from-binary: %.words-sorted.binary
	rm -f $*.hybrid.prefixes
	$(MAKE) $*.hybrid.prefixes
	$(CS_BIN_DIR)/buildIndex -b $*.hybrid.prefixes -f BINARY HYB $*.ANY_SUFFIX_WITHOUT_DOT
	  2> $*.hybrid.build-index-errors | tee $*.hybrid.build-index-log
	ln -sf $*.hybrid $@

# Build a hybrid (HYB) index from an ASCII words file.
#
# TODO(Hannah): Potential problem in $(DB_PREFIX).hybrid.prefixes_2: can contain ' or
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
	  --stxxl-path=$(DATA_DIR) \
	  --stxxl-disk-file=$(DB_PREFIX).stxxl.disk \
	  --binary-words-file=$@
	rm -f stxxl.*

# Build fuzzy search data structure + clustering.
fuzzysearch:
	# Remove all : words from the vocabulary (but save it, so that we have
	# the original one in the end, including words like :filter:... which
	# are needed when writing the output in binary).
	# cp $(DB_PREFIX).vocabulary $(DB_PREFIX).vocabulary.BACKUP
	$(LOCALE_POSIX); $(SORT) -c $(DB_PREFIX).vocabulary
	cat $(DB_PREFIX).vocabulary | $(PERL) -ne 'print unless /$(WORD_SEPARATOR_BACKEND)/;' > $(DB_PREFIX).vocabulary.TMP
	mv -f $(DB_PREFIX).vocabulary.TMP $(DB_PREFIX).vocabulary
	ls -alh $(DB_PREFIX).vocabulary
	@echo
	# Same for the new .vocabulary+frequencies.
	$(LOCALE_POSIX); cut -f2 $(DB_PREFIX).vocabulary+frequencies | sort -c -n
	cat $(DB_PREFIX).vocabulary+frequencies | $(PERL) -ne 'print unless /$(WORD_SEPARATOR_BACKEND)/;' > $(DB_PREFIX).vocabulary+frequencies.TMP
	mv -f $(DB_PREFIX).vocabulary+frequencies.TMP $(DB_PREFIX).vocabulary+frequencies
	ls -alh $(DB_PREFIX).vocabulary+frequencies
	@echo
	# Clustering.
	printf "\n\n*** BUILD CLUSTERING ***\n";
	$(CS_CODE_DIR)/fuzzysearch/buildFuzzySearchClusters \
	  -a 1 -s 1 -i 100 -o 1 -m 100 -q 100 \
	  $(shell $(PERL) -e 'print $(FUZZY_COMPLETION_MATCHING) ? "-c" : "";') \
	  --locale=$(LOCALE) \
	  $(shell $(PERL) -e 'print $(FUZZY_NORMALIZE_WORDS) ? "--normalize" : "";') \
	  $(shell $(PERL) -e 'print $(FUZZY_SEARCH_ALGORITHM) eq "simple" ? "-T" : "";') \
	  $(DB_PREFIX)

# Get CSV file from https://ad-research.cs.uni-freiburg.de (for example
# applications). If the file already exists, will not download it again.
CSV_FILE = $(DB_PREFIX)$(CSV_SUFFIX)
CSV_FILE_URL = "https://ad-research.cs.uni-freiburg.de/data/csv-files/$(notdir $(CSV_FILE)).bz2"
csv:
	@echo
	@if [ -f "$(CSV_FILE)" ]; then \
	  printf "\x1b[1mFile $(CSV_FILE) exists\x1b[0m\n"; fi
	@if [ ! -f "$(CSV_FILE)" ]; then \
	  echo "Downloading $(CSV_FILE_URL)"; \
	  wget -O $(CSV_FILE).bz2 $(CSV_FILE_URL); \
	  echo "Uncompressing $(CSV_FILE).bz2 (this can take a while)"; \
	  bunzip2 $(CSV_FILE).bz2; \
	  echo; ls -hl $(CSV_FILE); fi
	@echo

# SECTION 3: DEPRECATED STUDD


# make vocabulary with frequency (for SIGIR'08 docs.DB)
freqranks:
	$(LOCALE_POSIX); cut -f1 $(DB_PREFIX).words \
	  | uniq -c \
	  | $(SORT) -k1,1nr \
	  | $(PERL) -e 'while(<>){++$$c; /^\s*\d+\s*(\S+)\s*$$/; print "$$1\t$$c\n";}' \
	  > $(DB_PREFIX).vocabulary+freqranks
	tail $(DB_PREFIX).vocabulary+freqranks
	wc -l $(DB_PREFIX).vocabulary+freqranks

# Is this still working or needed?
parse-xml:
	$(MAKE) $(DB_PREFIX).parser
	./$(DB_PREFIX).parser $(PARSE_OPTS) $(DB_PREFIX).xml $(DB_PREFIX).docs-unsorted $(DB_PREFIX).words-unsorted.ascii

# Is this still working or needed?
enhance-words:
	$(LOCALE_POSIX); $(SORT) -k1,1 -k2,2n -k4,4n $(DB_PREFIX).words.enhancing.* \
	  | $(SORT) -m -k1,1 -k2,2n -k4,4n $(DB_PREFIX).words - \
	  > $(DB_PREFIX).words.enhanced
	mv -f $(DB_PREFIX).words.enhanced $(DB_PREFIX).words

# Is this still working or needed?
enhance-docs:
	$(MAKE) -C bin enhanceExcerpts
	$(PERL) -ne 's/$(WORD_SEPARATOR_BACKEND)$(WORD_SEPARATOR_BACKEND)[^\t]+//; print;' < $(DB_PREFIX).words.enhancing.spell > $(DB_PREFIX).words.enhancing.spell.tmp
	$(CS_DIR)/enhanceExcerpts $(DB_PREFIX).docs $(DB_PREFIX).docs.enhanced $(DB_PREFIX).words.enhancing.spell.tmp
	rm -f $(DB_PREFIX).words.enhancing.spell.tmp
	$(LOCALE_POSIX); $(SORT) -c -k1,1n $(DB_PREFIX).docs
	mv -f $(DB_PREFIX).docs.enhanced $(DB_PREFIX).docs
