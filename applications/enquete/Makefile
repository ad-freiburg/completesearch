include codebase/Makefile

# NOTE: For the old Completesearch UI, must parse with --old-words-format
# NOTE: and set the separators to : (both frontend and backend)
# NOTE: Outside of docker, remove the symbolic link to /completesearch

WORD_SEPARATOR_FRONTEND = :
WORD_SEPARATOR_BACKEND  = :

SHELL = /usr/local/bin/bash
WORDS_FORMAT = ASCII
ENCODING = utf8
LOCALE = en_US.utf8
ENABLE_FUZZY_SEARCH = 0
FUZZY_SEARCH_ALGORITHM = simple
FUZZY_USE_BASELINE = 1
HYB_PREFIX_LENGTH = 2
ENABLE_SYNONYM_SEARCH = 0
ENABLE_BINARY_SORT = 0
PARSE_EXTENDED_DTD = 0
NORMALIZE_WORDS = 1
VERBOSITY = 1
SHOW_QUERY_RESULT = 0
SORT = sort -T data -S 1G
SERVER_OPTIONS_SHORT = -Z
PARSER=$(CS_CODE_DIR)/parser/CsvParserMain
PORT = 8080

PARSER_OPTIONS = \
        --old-words-format \
	--full-text=abschnitt,text \
	--facets=berichtsteil \
	--show=berichtsteil,abschnitt \
	--excerpts=text \
	--base-name=$(DB) \
	--encoding=$(ENCODING) \
	--maps-directory=$(CS_CODE_DIR)/utility \
	$(shell perl -e 'print $(ENABLE_SYNONYM_SEARCH) ? "--read-synonym-groups" : "";') \
  	$(shell perl -e 'print $(NORMALIZE_WORDS) ? "--normalize-words" : "";') \
  	$(shell perl -e 'print $(PARSE_EXTENDED_DTD) ? "--parse-extended-dtd" : "";')

test-pall-csv:
	make pclean-all pall
