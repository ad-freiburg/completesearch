include codebase/Makefile

SHELL = /usr/local/bin/bash
ENCODING = utf8
LOCALE = en_US.utf8
ENABLE_FUZZY_SEARCH = 0 
FUZZY_SEARCH_ALGORITHM = simple
FUZZY_USE_BASELINE = 1
HYB_PREFIX_LENGTH = 3
ENABLE_SYNONYM_SEARCH = 0
ENABLE_BINARY_SORT = 0
PARSE_EXTENDED_DTD = 0
NORMALIZE_WORDS = 0
VERBOSITY = 1
DOCUMENT_ROOT = ""
DISABLE_CDATA_TAGS = 1
SHOW_QUERY_RESULT = 0
USE_SUFFIX_FOR_EXACT_QUERY = 1
CLEANUP_BEFORE_PROCESSING = 0
SORT = sort -T data -S 1G
MULTIPLE_TITLE = 1
WORD_SEPARATOR_FRONTEND = :
# WORD_SEPARATOR_BACKEND should not be quoted, since this results in problem
# when using regexes for generating the hybrid prefix list.
WORD_SEPARATOR_BACKEND  = :
# WORD_SEPARATOR_BACKEND  = !
INFO_DELIMITER = ''
# DB = data/vvz-test
# PORT = 8182
DB = data/vvz
PORT = 8181

# 2017-01-11: Current CSV columns are:
#
# id	lectureid	title	lecturers	dates	description	examination_rules	subject_of_study	category	his_category	semester	languages	type	returnData
#
FILTER=lecturer,subject_of_study,category,type,language

PARSER_OPTIONS = \
  --base-name=$(DB) \
  --encoding=$(ENCODING) \
  --maps-directory=. \
  --word-part-separator-backend=${WORD_SEPARATOR_BACKEND} \
  --full-text=$(FILTER),title \
  --show=lectureid";"returnData \
  --filter-plus=$(FILTER) \
  --facets=$(FILTER),his_category,examination_rule,semester \
  --facetid=$(FILTER),his_category,examination_rule,semester \
  --within-field-separator=\| --no-show-facet-prefix=\"*\" \
  --allow-multiple-items=lecturer,dates,examination_rule,subject_of_study,language \
  --field-format=returnData:json \

csv:
	perl -ne 's/","/\t/g; s/"//g; print' data/vvz.csv.original > data/vvz.csv
	head -2 data/vvz.csv
