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
ENABLE_FUZZY_SEARCH = 0
FUZZY_SEARCH_ALGORITHM = simple
FUZZY_USE_BASELINE = 1
HYB_PREFIX_LENGTH = 3
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
# new UI, we want words like :facet:... and not :ct:... And don't forget the
# facetids, they are important for efficiency (when adding a single facet to a
# query).
PARSER_OPTIONS = \
	--full-text=title,author,venue,year,type \
	--show=title,author,venue,year,type \
	--facets=author,venue,year,type \
	--facetids=author,venue,year,type \
	--within-field-separator=\# \
	--allow-multiple-items=author \
	--no-show-prefix=\* \
	--base-name=$(DB_PREFIX) \
        --encoding=$(ENCODING) \
	--csv-suffix=$(CSV_SUFFIX) \
	--maps-directory=$(MAPS_DIR) \
	$(shell perl -e 'print $(ENABLE_SYNONYM_SEARCH) ? "--read-synonym-groups" : "";') \
  	$(shell perl -e 'print $(NORMALIZE_WORDS) ? "--normalize-words" : "";') \
  	$(shell perl -e 'print $(PARSE_EXTENDED_DTD) ? "--parse-extended-dtd" : "";')

# The following target can be used to regenerate the CSV file. It's just here
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
example.csv:
	curl -Gs "https://qlever.cs.uni-freiburg.de/api/wikidata" --data-urlencode 'query=PREFIX schema: <http://schema.org/> PREFIX wd: <http://www.wikidata.org/entity/> PREFIX wdt: <http://www.wikidata.org/prop/direct/> PREFIX rdfs: <http://www.w3.org/2000/01/rdf-schema#> PREFIX wikibase: <http://wikiba.se/ontology#> SELECT ?film_id ?wikipedia ?film ?description (MIN(?date) as ?date) (SAMPLE(?sitelinks) as ?sitelinks) (GROUP_CONCAT(DISTINCT ?director;SEPARATOR=";") AS ?directors) (GROUP_CONCAT(DISTINCT ?genre;SEPARATOR=";") AS ?genres) WHERE { ?film_id wdt:P31 wd:Q11424 . ?film_id @en@rdfs:label ?film . ?film_id @en@schema:description ?description . ?film_id wdt:P136 ?genre_id . ?genre_id @en@rdfs:label ?genre . ?film_id wdt:P57 ?director_id . ?director_id @en@rdfs:label ?director . ?film_id wdt:P577 ?date . ?film_id ^schema:about ?wikipedia . ?wikipedia schema:isPartOf <https://en.wikipedia.org/> . ?film_id ^schema:about/wikibase:sitelinks ?sitelinks . } GROUP BY ?film_id ?wikipedia ?film ?description ORDER BY DESC(?sitelinks)' --data-urlencode 'action=tsv_export' | sed 's/@en//g;s/T00:00:00//;s/\^\^\S\+//g;s/;$$//;s/;\t/\t/g;s/["<>]//g' > $@

