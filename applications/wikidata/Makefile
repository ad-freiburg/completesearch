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
	--full-text=entity,type,abstract \
	--show=wikidata,wikipedia,entity,type \
	--excerpts=abstract \
	--facets=type \
	--facetids=type \
	--within-field-separator=\# \
	--allow-multiple-items=type \
	--base-name=$(DB_PREFIX) \
	--csv-suffix=$(CSV_SUFFIX) \
        --encoding=$(ENCODING) \
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
wikidata.csv:
	printf "wikidata\twikipedia\tentity\ttype\tabstract\tsitelinks\n" > $@
	curl -Gs http://vulcano.informatik.privat:7009 \
	 --data-urlencode 'query=PREFIX rdfs: <http://www.w3.org/2000/01/rdf-schema#> PREFIX schema: <http://schema.org/> PREFIX wd: <http://www.wikidata.org/entity/> PREFIX wdt: <http://www.wikidata.org/prop/direct/> PREFIX wikibase: <http://wikiba.se/ontology#> SELECT ?wikidata ?wikipedia ?name (GROUP_CONCAT(?type;SEPARATOR="#") AS ?types) ?abstract ?sitelinks WHERE { { SELECT DISTINCT ?wikidata ?wikipedia ?type ?abstract WHERE { { ?wikidata ^schema:about ?wikipedia . ?wikipedia schema:abstract ?abstract . ?wikidata wdt:P31 ?type_id . ?type_id @en@rdfs:label ?type_id_label } UNION { ?wikidata ^schema:about ?wikipedia . ?wikipedia schema:abstract ?abstract . ?wikidata wdt:P31 ?tmp . ?tmp wdt:P279+ ?type_id . ?type_id @en@rdfs:label ?type } } } ?wikidata @en@rdfs:label ?name . ?wikidata ^schema:about/wikibase:sitelinks ?sitelinks } GROUP BY ?wikidata ?wikipedia ?name ?abstract ?sitelinks ORDER BY DESC(?sitelinks)' \
	  --data-urlencode 'action=tsv_export' \
	  | sed 's/@en//g;s/T00:00:00//;s/\^\^\S\+//g;s/;$$//;s/;\t/\t/g;s/["<>]//g;s/\(@ \)\+/@ /g;s/\t@ /\t/;s/ @\t/\t/' >> $@



wikidata.csv.PEOPLE:
	printf "wikidata\twikipedia\tperson\toccupation\tabstract\tsitelinks\n" > $@
	curl -Gs https://qlever.cs.uni-freiburg.de/api/wikidata-plus \
	  --data-urlencode 'query=PREFIX rdfs: <http://www.w3.org/2000/01/rdf-schema#> PREFIX schema: <http://schema.org/> PREFIX wd: <http://www.wikidata.org/entity/> PREFIX wdt: <http://www.wikidata.org/prop/direct/> PREFIX wikibase: <http://wikiba.se/ontology#> SELECT ?person_id ?wikipedia ?person (GROUP_CONCAT(?occupation;SEPARATOR="#") AS ?occupations) ?abstract ?sitelinks WHERE { { SELECT ?person_id ?wikipedia ?person ?abstract ?sitelinks WHERE { ?person_id wdt:P31 wd:Q5 . ?person_id ^schema:about ?wikipedia . ?wikipedia schema:abstract ?abstract . ?person_id @en@rdfs:label ?person . ?person_id ^schema:about/wikibase:sitelinks ?sitelinks } } ?person_id wdt:P106/@en@rdfs:label ?occupation } GROUP BY ?person_id ?wikipedia ?person ?abstract ?sitelinks ORDER BY DESC(?sitelinks)' \
	  --data-urlencode 'action=tsv_export' | sed 's/@en//g;s/T00:00:00//;s/\^\^\S\+//g;s/;$$//;s/;\t/\t/g;s/["<>]//g;s/\(@ \)\+/@ /g' >> $@
