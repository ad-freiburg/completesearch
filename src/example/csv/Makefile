include ../../Makefile

LOCALE = de_DE.iso88591
SORT = sort -T sort-tmp -S 1G
WORDS_FORMAT = ASCIISHELL = /usr/local/bin/bash
ENCODING = iso88591
LOCALE = en_US.utf8
ENABLE_FUZZY_SEARCH = 1 
FUZZY_SEARCH_ALGORITHM = simple
FUZZY_USE_BASELINE = 1
HYB_PREFIX_LENGTH = 3
ENABLE_SYNONYM_SEARCH = 0
ENABLE_BINARY_SORT = 0
PARSE_EXTENDED_DTD = 0
NORMALIZE_WORDS = 1
VERBOSITY = 1
SHOW_QUERY_RESULT = 0
SORT = sort -T data -S 1G
WORDS_FORMAT = BINARY
PARSER=$(CS_CODE_DIR)/parser/CsvParserMain
DB = example-input

PARSER_OPTIONS = \
	--full-text=Autor,Titel,Beschreibung --show=Autor,Titel,Typ,Verlag,Jahr,Preis \
	--filter=Autor,Titel --facets=Autor,Jahr,Verlag --excerpts=Beschreibung \
	--within-field-separator=\; \
	--base-name=$(DB) encoding=$(ENCODING) \
	--maps-directory=$(CS_CODE_DIR)/utility \
	$(shell perl -e 'print $(ENABLE_SYNONYM_SEARCH) ? "--read-synonym-groups" : "";') \
  	$(shell perl -e 'print $(NORMALIZE_WORDS) ? "--normalize-words" : "";') \
  	$(shell perl -e 'print $(PARSE_EXTENDED_DTD) ? "--parse-extended-dtd" : "";')

test-pall-csv:
	make pclean-all pall
