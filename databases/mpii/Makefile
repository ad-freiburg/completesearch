CS_CODE_DIR = ../../codebase
include $(CS_CODE_DIR)/Makefile
#ENCODING = utf8
#LOCALE = de_DE.utf8
PORT = 8889
ENCODING = iso
LOCALE = de_DE.iso88591
DB = data/mpii
ENABLE_FUZZY_SEARCH = 0
ENABLE_SYNONYM_SEARCH = 0
ENABLE_BINARY_SORT = 0
SORT = sort -T sort-tmp -S 1G
WORDS_FORMAT = BINARY
PARSER = $(CS_CODE_DIR)/parser/CsvParserMain
SERVER_OPTIONS_SHORT = -s SSSS
PARSER_OPTIONS = \
--base-name=$(DB) --encoding=$(ENCODING) \
--full-text=url,title,fulltext,h0,h1,h2,h3,h4 --show=url,title \
--score=h0:250,h1:150,h2:120,h3:70,h4:20 \
--excerpts=fulltext
# files that are created by target pall (parsing and indexing)
DB_FILES = docs* hybrid* log make-begin make-end parse-log vocabulary words*	 
# CRAWLER ---------------------------------------------------------------------
# To start a new download, add a new "<link>"-tag to the crawl.config-file.
# Examples and descriptions of the tag can be found in crawl.config
# To update a previously crawled site, set status="u" in the link-tag
# of that site
# If you want to recrawl a certain site, you have to set status="d"
# AND either delete the contents of the folder to which the site was downloaded
# or create a new folder and set the folder-attribute of the link-tag 
# to the new folder
# If you want to prevent the crawler from making any changes to a mirrored
# site, set status="n" in this sites' link tag
CRAWLER_DIRECTORY = $(shell pwd)/crawler/
CRAWLER_CONFIG = $(shell pwd)/crawl.config
CRAWLER_ARGS = -i -b
#CRAWLER_ARGS = -i
CSV_DIRECTORY = $(shell pwd)/data

# Overwrite target, so that docs are not only sorted but also the output from
# the CSV parser is converted to the format expected by the old UI (in
# particular, the UI from www.mpi-inf.mpg.de).
%.docs-sorted: %.docs-unsorted
	$(LOCALE_POSIX); $(SORT) -b -k1,1n $*.docs-unsorted \
	  | perl -ne 'if (/<show field="url">(.*?)<\/show><show field="title">(.*?)<\/show>/) { $$u = $$1; $$t = $$2; }; s/\tu:[^\t]+\tt:[^\t]+/\tu:$$u\tt:$$t/; print; ' \
	  > $*.docs-sorted

geek3:
	ssh hannah@contact.mpi-inf.mpg.de "cd completesearch/databases/mpii; rm -f mpii.hybrid mpii.vocabulary mpii.docs.DB; wget http://stromboli.informatik.uni-freiburg.de/mpii/mpii.hybrid; wget http://stromboli.informatik.uni-freiburg.de/mpii/mpii.vocabulary; wget http://stromboli.informatik.uni-freiburg.de/mpii/mpii.docs.DB; make geek3 </dev/null;"

copy-mpii.OLD:
	scp mpii.docs-sorted mpii.words-sorted.ascii hannah@contact.mpi-inf.mpg.de:completesearch/databases/mpii
	# scp mpii.hybrid mpii.docs.DB mpii.vocabulary hannah@contact.mpi-inf.mpg.de

cleanDB:
	for dbFile in $(DB_FILES); do\
		rm $(DB).$$dbFile; \
	done	
crawl:
	$(CRAWLER_DIRECTORY)CrawlerMain -s $(CRAWLER_DIRECTORY) -c $(CRAWLER_CONFIG) -d $(CSV_DIRECTORY) $(CRAWLER_ARGS)
