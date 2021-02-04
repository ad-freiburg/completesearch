# CONFIGURATION OF PRECOMPUTATION for a particular application
#
# This file is included from applications/Makefile via include $(DB)/Makefile

CSV_SUFFIX = .tsv
LOCALE = en_us.utf8
WORDS_FORMAT = ASCII
WORD_SEPARATOR_FRONTEND = :
WORD_SEPARATOR_BACKEND  = :
SHELL = /usr/local/bin/bash
ENCODING = utf8
ENABLE_CORS = 1
DOCUMENT_ROOT = /applications/$(DB)
ENABLE_FUZZY_SEARCH = 0
FUZZY_SEARCH_ALGORITHM = simple
FUZZY_USE_BASELINE = 1
HYB_PREFIX_LENGTH = 2
ENABLE_SYNONYM_SEARCH = 0
ENABLE_BINARY_SORT = 1
PARSE_EXTENDED_DTD = 0
NORMALIZE_WORDS = 0
VERBOSITY = 1
SHOW_QUERY_RESULT = 0
SORT = sort -T data -S 1G
SERVER_OPTIONS_SHORT = -Z
PARSER=$(CS_CODE_DIR)/parser/CsvParserMain
PORT = 8080

# Previous version used --old-words-format, which is needed for old UI. For the
# new UI, we want words like :facet:... and not :ct:...
PARSER_OPTIONS = \
	--full-text=film,abstract,date,director,genre \
	--show=wikipedia,film,date,director \
	--excerpts=abstract,genre \
	--filter=director \
	--facets=director,genre \
	--facetids=director,genre \
	--within-field-separator=\# \
	--allow-multiple-items=director,genre \
	--base-name=$(DB_PREFIX) \
	--csv-suffix=$(CSV_SUFFIX) \
        --encoding=$(ENCODING) \
	--maps-directory=$(MAPS_DIR) \
	$(shell perl -e 'print $(ENABLE_SYNONYM_SEARCH) ? "--read-synonym-groups" : "";') \
  	$(shell perl -e 'print $(NORMALIZE_WORDS) ? "--normalize-words" : "";') \
  	$(shell perl -e 'print $(PARSE_EXTENDED_DTD) ? "--parse-extended-dtd" : "";')
	
# The following target can be used to regenerate the TSV file. It's just here
# for archival purposes, to try out the example with CompleteSearch, there is no
# need to regenerate this file.
#
# The target assumes that a QLever instance for Wikidata is running under
# qlever.cs.uni-freiburg.de/api/wikidata. It only works from within the Docker
# container because of the include in the first line of this Makefile. Here is
# the command line to run it from the Docker container.
#
# docker run --rm -it -v $(pwd)/applications/example:/configuration completesearch.example -c "make example.csv"
#
# The sed command in the end just makes the file nicer. The header line needs to
# be added manually.
movies.tsv:
	printf "wikidata\twikipedia\tfilm\tabstract\tdate\tsitelinks\tdirector\tgenre\n" > data/$@
	curl -Gs "https://qlever.cs.uni-freiburg.de/api/wikidata-plus" \
	  --data-urlencode 'query=PREFIX schema: <http://schema.org/> PREFIX wd: <http://www.wikidata.org/entity/> PREFIX wdt: <http://www.wikidata.org/prop/direct/> PREFIX rdfs: <http://www.w3.org/2000/01/rdf-schema#> PREFIX wikibase: <http://wikiba.se/ontology#> SELECT ?film_id ?wikipedia ?film ?abstract (MIN(?date) as ?date) (SAMPLE(?sitelinks) as ?sitelinks) (GROUP_CONCAT(DISTINCT ?director;SEPARATOR="#") AS ?directors) (GROUP_CONCAT(DISTINCT ?genre;SEPARATOR="#") AS ?genres) WHERE { ?film_id wdt:P31 wd:Q11424 . ?film_id @en@rdfs:label ?film . ?film_id wdt:P136 ?genre_id . ?genre_id @en@rdfs:label ?genre . ?film_id wdt:P57 ?director_id . ?director_id @en@rdfs:label ?director . ?film_id wdt:P577 ?date . ?film_id ^schema:about ?wikipedia . ?wikipedia schema:abstract ?abstract . ?film_id ^schema:about/wikibase:sitelinks ?sitelinks } GROUP BY ?film_id ?wikipedia ?film ?abstract ORDER BY DESC(?sitelinks)' \
	  --data-urlencode 'action=tsv_export' \
	  | sed 's/@en//g;s/T00:00:00//;s/\^\^\S\+//g;s/#$$//;s/#\t/\t/g;s/["<>]//g;s/\(@ \)\+/@ /g;s/\t@ /\t/;s/ @\t/\t/;' \
	  >> data/$@
