# ---------------------------------------------------------
# -----------------------------------------------------------------------------
# New version of this Makefile made on 20th of June 2010 by Björn
# Refactoring from compile/run/etc shortcuts towards a real chain.
# ----
# New version of this Makefile made on 25th of July 2010 by Björn
# Refactoring into a Makefile that supports split data and code.
# ----
# New version of this Makefile made on 4th of May 2011 by Björn
# Refactoring the whole semantic-wikipedia folder into something
# similar to the codebase folder. Changing it towards a base
# featuring subfolders for all important features.
# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------

# -----------------------------------------------------------------------------
# Variables below.
# -----------------------------------------------------------------------------

# -----------------------------------------------------------------------------
# The following variables HAVE to be changed according to your
# local settings.
# -----------------------------------------------------------------------------
SEM_WIK_DIR = /home/$(USER)/completesearch/databases/semantic-wikipedia
CS_CODE_DIR = $(SEM_WIK_DIR)/codebase
include $(CS_CODE_DIR)/Makefile
# Directory where input + output files are located.
DATA_DIRECTORY = /nfs/raid1/buchholb/semantic-wikipedia-data
#DATA_DIRECTORY = /mnt/buchholb/semantic-wikipedia-data
# Port on which the CompletionSever will run.  
PORT = 8887
# -----------------------------------------------------------------------------

# -----------------------------------------------------------------------------
# The following variables can be changed per build 
# depending on settings you need momentarily.
# -----------------------------------------------------------------------------
# Compiler command
CXX = g++ -O6 -Wall -DLOGLEVEL=4 -I${GTEST_DIR}/include
# CXX = g++ -O6 -Wall -DLOGLEVEL=3 -I${GTEST_DIR}/include
# CXX = g++ -O6 -Wall -DLOGLEVEL=2 -DNDEBUG -I${GTEST_DIR}/include
# CXX = g++ -pg -Wall -I${GTEST_DIR}/include
# CXX = g++ -g -Wall -DLOGLEVEL=4 -I${GTEST_DIR}/include

# Database to build
DB = $(DATA_DIRECTORY)/$(DBTAIL)
# Change this line or set it when calling make to change the DB to build
DBTAIL = $(DBFULL)
DBSCIENT = semantic-wikipedia-scientists
DBTEST = semantic-wikipedia-test
DBREL = semantic-wikipedia-relations
DBFULL = semantic-wikipedia-full-sep11
DBTEST = semantic-wikipedia-test
DBMEDICINE = semantic-wikipedia-medicine
# -----------------------------------------------------------------------------

# -----------------------------------------------------------------------------
# The following variables may be changed but usually do not have to. 
# -----------------------------------------------------------------------------
YAGODIR = /nfs/raid1/buchholb/yago
ONTOLOGY = ontology
ONTOLOGY_ASCII = $(DATA_DIRECTORY)/ontology.ascii
ONTOLOGY_VOCABULARY = $(DATA_DIRECTORY)/ontology.vocabulary
ONTOLOGY_ENTITY_LIST = $(DATA_DIRECTORY)/ontology.entity-list
ONTOLOGY_ABSTRACTNESS_COUNTS = $(DATA_DIRECTORY)/ontology.abstractness-counts
EVALBASE = tools/evaluation
SORT = sort -T $(DATA_DIRECTORY)
INDEX_PREFIX_LENGTH = 4
MAX_SIZE_HISTORY = 2048M
ENABLE_BINARY_SORT = 1
BOOSTDIR = /nothing/ 
DEPLOY_DIR = alicudi:/local/data/broccoli/$(USER)
StopWordsFile = $(SEM_WIK_DIR)/semantic-wikipedia.stop-words
RedirectMapInput = $(DATA_DIRECTORY)/$(DBFULL)
RedirectMap = $(DATA_DIRECTORY)/semantic-wikipedia.redirect-map
YagoLeaves = $(DATA_DIRECTORY)/semantic-wikipedia.yago-leaves
YagoPaths = $(DATA_DIRECTORY)/semantic-wikipedia.yago-paths
YagoFacts = $(DATA_DIRECTORY)/semantic-wikipedia.yago-facts
YagoRelations = $(DATA_DIRECTORY)/semantic-wikipedia.yago-relations
WordnetSynonyms = $(DATA_DIRECTORY)/semantic-wikipedia.wordnet-synonyms
PERSONPATH = "^:e:entity:physicalentity:object:whole:livingthing:organism:person:" 

DefaultParseOptions = --section-headers-to-skip $(SectionsToSkip) --redirect-map $(RedirectMap) --wordnet-synonyms $(WordnetSynonyms) --pronouns-file-name $(PronounsFile) --max-header-level 0 --abstractness-count $(ONTOLOGY_ABSTRACTNESS_COUNTS)
ParseOptionsSUSI = $(DefaultParseOptions) --yago-facts $(YagoFacts) --yago-paths $(YagoPaths)
ParseOptionsWithRelations = $(ParseOptionsSUSI) --yago-relations $(YagoRelations) --only-write-entities-from-ontology
ParseOptionsForNews = --news --section-headers-to-skip $(SectionsToSkip) --redirect-map $(RedirectMap) --wordnet-synonyms $(WordnetSynonyms) --pronouns-file-name $(PronounsFileDE) --max-header-level 0 --yago-facts $(YagoFacts) --yago-paths $(YagoPaths) --yago-relations $(YagoRelations) --only-write-entities-from-ontology
ParseOptionsSemsearch = --section-headers-to-skip $(SectionsToSkip) --redirect-map $(RedirectMap) --pronouns-file-name $(PronounsFile) --max-header-level 0 --ontology-entity-list $(ONTOLOGY_ENTITY_LIST) --stop-words $(StopWordsFile) --only-write-entities-from-ontology --do-not-write-paths
ParseOptionsSemsearchQuick = --section-headers-to-skip $(SectionsToSkip) --pronouns-file-name $(PronounsFile) --max-header-level 0 --ontology-entity-list $(ONTOLOGY_ENTITY_LIST) --stop-words $(StopWordsFile) --only-write-entities-from-ontology --do-not-write-paths
ParseOptionsOutputCategories = --output-categories --pronouns-file-name $(PronounsFile) --max-header-level 0 --ontology-entity-list $(ONTOLOGY_ENTITY_LIST) --only-write-entities-from-ontology --do-not-write-paths
PronounsFile = parser/anaphora-pronouns-list.en
PronounsFileDE = parser/anaphora-pronouns-list.de
SectionsToSkip = parser/semantic-wikipedia.sections_to_skip

ArticleExtractor = $(YAGODIR)/facts/type/ArticleExtractor.txt
IsAExtractor = $(YAGODIR)/facts/type/IsAExtractor.txt
WordNetLinks = $(YAGODIR)/facts/subClassOf/WordNetLinks.txt
ConceptLinker = $(YAGODIR)/facts/subClassOf/ConceptLinker.txt

GLOBAL_OBJECTS = $(UTILS_DIR)/Conversions.o
GTEST_DIR = $(CS_CODE_DIR)/gtest
LIBGTEST_A = $(CS_CODE_DIR)/../databases/semantic-wikipedia/libgtest.a
CXX_TEST = $(CXX) -I${GTEST_DIR}/include
TEST_INCLUDE = $(LIBGTEST_A) -lpthread

UTILS_DIR = $(CS_CODE_DIR)/semantic-wikipedia-utils

# -----------------------------------------------------------------------------

# -----------------------------------------------------------------------------
# General rules by name patterns.
# -----------------------------------------------------------------------------
		
# HEADERS = $(wildcard ../utils/*.h)
HEADERS = $(wildcard ../codebase/semantic-wikipedia-utils/*.h)

#%Test.o: %Test.cpp $(HEADERS)
#	$(CXX_TEST) -c $<
	
#%.o: %.cpp $(HEADERS)
#	$(CXX) -c $<	

# General rules for binaries. While this makes the Makefile cleaner,
# they are clearly inferior to specific rules, because too many objects
# are passed to the linker
.PRECIOUS: %.o

%Main: %Main.o $(OBJECTS) $(GLOBAL_OBJECTS) $(HEADERS)
	$(CXX) -o $@ $< $(OBJECTS) $(GLOBAL_OBJECTS) $(LIBS_INCLUDED)

%Test: %Test.o $(OBJECTS) $(HEADERS) $(GLOBAL_OBJECTS)
	$(CXX_TEST) -o $@ $< $(OBJECTS) $(GLOBAL_OBJECTS) $(LIBS_INCLUDED) $(TEST_INCLUDE)
	
# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------
# Start of target declarations.
# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------

# -----------------------------------------------------------------------------
# Targets related to building binaries from sources.
# -----------------------------------------------------------------------------
$(UTILS_DIR)/Conversions.o:
	make -C $(UTILS_DIR)/ Conversions.o


.PHONY = all build-semsearch test-semsearch clean-semsearch do-build do-test do-clean

all: $(LIBGTEST_A) build-semsearch test-semsearch lint-all

perf-tests: $(DB)-full.docs-offsets
	$(MAKE) -C server do-perf-test
	
build-semsearch: 
	$(MAKE) -C $(UTILS_DIR) do-build
	$(MAKE) -C tools do-build
	$(MAKE) -C ontology-manager do-build
	$(MAKE) -C parser do-build
	$(MAKE) -C index-builder do-build
	$(MAKE) -C server do-build

test-semsearch:
	$(MAKE) -C $(UTILS_DIR) do-test
	$(MAKE) -C tools do-test
	$(MAKE) -C ontology-manager do-test
	$(MAKE) -C parser do-test
	$(MAKE) -C index-builder do-test
	$(MAKE) -C server do-test

clean-semsearch:
	rm -f $(LIBGTEST_A)
	$(MAKE) -C $(UTILS_DIR) do-clean
	$(MAKE) -C tools do-clean
	$(MAKE) -C ontology-manager do-clean
	$(MAKE) -C parser do-clean
	$(MAKE) -C index-builder do-clean
	$(MAKE) -C server do-clean
	
lint-all:
	$(MAKE) -C $(UTILS_DIR)/ lint
	$(MAKE) -C parser/ lint
	$(MAKE) -C server/ lint
	$(MAKE) -C ontology-manager/ lint
	$(MAKE) -C tools/ lint
	$(MAKE) -C tools/evaluation lint
	$(MAKE) -C index-builder/ lint
	$(MAKE) -C trials/ lint
	$(MAKE) -C trials/log-implementations/ lint
	
$(LIBGTEST_A):
	g++ -I${GTEST_DIR}/include -I${GTEST_DIR} -c ${GTEST_DIR}/src/gtest-all.cc
	g++ -I${GTEST_DIR}/include -I${GTEST_DIR} -c ${GTEST_DIR}/src/gtest_main.cc
	ar -rv $(LIBGTEST_A) gtest-all.o gtest_main.o

# -----------------------------------------------------------------------------
# Targets related to execution / Make-Chain.
# -----------------------------------------------------------------------------

# -----------------------------------------------------------------------------
# Targets for the new semantic search.
# -----------------------------------------------------------------------------

$(DATA_DIRECTORY)/semantic-wikipedia.yago-paths-with-underscores:
	ontology-manager/OntologyManagerMain -p $(DATA_DIRECTORY) $(ONTOLOGY)

rebuild-ontology: build-semsearch $(DATA_DIRECTORY)/wordnet-disambiguation-map $(WordnetSynonyms) $(RedirectMap)
	ontology-manager/OntologyManagerMain --ontology-ascii-output $(ONTOLOGY_ASCII) --redirect-map $(RedirectMap) -w $(DATA_DIRECTORY)/wordnet-disambiguation-map $(ONTOLOGY)

disambiguation-map: $(DATA_DIRECTORY)/wordnet-disambiguation-map

$(DATA_DIRECTORY)/wordnet-disambiguation-map: $(DATA_DIRECTORY)/semantic-wikipedia.yago-paths-with-underscores
	make -C codebase/
	make -C ontology-manager/ SimpleWordnetCategoryDisambiguator
	ontology-manager/SimpleWordnetCategoryDisambiguator -o $@ --paths-file $(DATA_DIRECTORY)/semantic-wikipedia.yago-paths-with-underscores --index-base $(DATA_DIRECTORY)/$(DBFULL) 

$(ONTOLOGY_ASCII): $(DATA_DIRECTORY)/wordnet-disambiguation-map $(RedirectMap) $(WordnetSynonyms)
	ontology-manager/OntologyManagerMain --ontology-ascii-output $(ONTOLOGY_ASCII) --redirect-map $(RedirectMap) -w $(DATA_DIRECTORY)/wordnet-disambiguation-map $(ONTOLOGY) 

$(ONTOLOGY_ABSTRACTNESS_COUNTS): $(ONTOLOGY_ASCII)
	grep :r:is-a $(ONTOLOGY_ASCII) | cut -f5 | sort | uniq -c | sed 's/^\s*//g' | sed 's/ /\t/' > $(ONTOLOGY_ABSTRACTNESS_COUNTS)

$(ONTOLOGY_ENTITY_LIST): $(ONTOLOGY_ASCII)
	ontology-manager/CreateOntologyEntityListMain -i $(ONTOLOGY_ASCII) -o $@

parse-semsearch: build-semsearch $(RedirectMap) $(ONTOLOGY_ENTITY_LIST) $(ONTOLOGY_ABSTRACTNESS_COUNTS) $(DB).xml 
	parser/SemanticWikipediaParserMain $(ParseOptionsSemsearch) $(DB)
	
parse-semsearch-quick: build-semsearch $(ONTOLOGY_ENTITY_LIST) $(ONTOLOGY_ABSTRACTNESS_COUNTS) $(DB).xml
	parser/SemanticWikipediaParserMain $(ParseOptionsSemsearchQuick) $(DB)
		
parse-for-categories: build-semsearch $(ONTOLOGY_ENTITY_LIST) $(ONTOLOGY_ABSTRACTNESS_COUNTS) $(DB).xml
	parser/SemanticWikipediaParserMain $(ParseOptionsOutputCategories) $(DB)

%.block-boundaries: %.vocabulary
	cut $*.vocabulary -c1-$(INDEX_PREFIX_LENGTH) | $(SORT) -u > $*.block-boundaries
	
%.vocabulary: %.words-by-contexts.ascii
	index-builder/VocabularyBuilderMain $*.words-by-contexts.ascii

$(ONTOLOGY_VOCABULARY): $(ONTOLOGY_ASCII)
	index-builder/VocabularyBuilderMain --ontology-file $(ONTOLOGY_ASCII) --ontology-output $(ONTOLOGY_VOCABULARY)

construct-vocabulary: $(ONTOLOGY_ASCII)
	index-builder/VocabularyBuilderMain --ontology-file $(ONTOLOGY_ASCII) --ontology-output $(ONTOLOGY_VOCABULARY) $(wildcard $(DB)-*.words-by-contexts.ascii)

$(ONTOLOGY_INDEX): $(ONTOLOGY_ASCII) $(ONTOLOGY_VOCABULARY) $(ONTOLOGY_ABSTRACTNESS_COUNTS)
	index-builder/IndexBuilderMain --ontology-vocab $(ONTOLOGY_VOCABULARY) --ontology-input $(ONTOLOGY_ASCII) --class-instances-counts $(ONTOLOGY_ABSTRACTNESS_COUNTS) --ontology-blocks $(ONTOLOGY_VOCABULARY)

build-ontology-index: $(ONTOLOGY_INDEX)
	
build-text-index: $(ONTOLOGY_VOCABULARY) $(patsubst %.words-by-contexts.ascii,%.vocabulary,$(wildcard $(DB)-*.words-by-contexts.ascii)) $(patsubst %.words-by-contexts.ascii,%.block-boundaries,$(wildcard $(DB)-*.words-by-contexts.ascii)) $(patsubst %.words-by-contexts.ascii,%.docs-offsets,$(wildcard $(DB)-*.words-by-contexts.ascii))
	index-builder/IndexBuilderMain --ontology-vocab $(ONTOLOGY_VOCABULARY) --ontology-blocks $(ONTOLOGY_VOCABULARY) $(patsubst %.words-by-contexts.ascii,%,$(wildcard $(DB)-*.words-by-contexts.ascii))
	
build-index: $(ONTOLOGY_ASCII) $(ONTOLOGY_VOCABULARY) $(ONTOLOGY_ABSTRACTNESS_COUNTS) $(patsubst %.words-by-contexts.ascii,%.vocabulary,$(wildcard $(DB)-*.words-by-contexts.ascii)) $(patsubst %.words-by-contexts.ascii,%.block-boundaries,$(wildcard $(DB)-*.words-by-contexts.ascii)) $(patsubst %.words-by-contexts.ascii,%.docs-offsets,$(wildcard $(DB)-*.words-by-contexts.ascii))
	index-builder/IndexBuilderMain --ontology-vocab $(ONTOLOGY_VOCABULARY) --ontology-input $(ONTOLOGY_ASCII) --class-instances-counts $(ONTOLOGY_ABSTRACTNESS_COUNTS) --ontology-blocks $(ONTOLOGY_VOCABULARY) $(patsubst %.words-by-contexts.ascii,%,$(wildcard $(DB)-*.words-by-contexts.ascii)) 

%.docs-offsets: %.docs-by-contexts.ascii
	server/ExcerptsOffsetsMain $*

	
# -----------------------------------------------------------------------------
# Targets for the old CompleteSearch-based smenatic search.
# -----------------------------------------------------------------------------

warmup:
	curl "http://localhost:${PORT}/?q=:wikiDoc:*"
	curl "http://localhost:${PORT}/?q=:ec:*"
	
deploy-index:
	scp -c arcfour $(DB).hybrid $(DEPLOY_DIR)
	scp -c arcfour $(DB).docs.DB $(DEPLOY_DIR)
	scp -c arcfour $(DB).vocabulary $(DEPLOY_DIR)

deploy: deploy-index
	scp $(CS_CODE_DIR)/server/startCompletionServer $(DEPLOY_DIR)

index-all:
	$(MAKE) sort
	$(MAKE) index 

%.hybrid.from-binary: %.words-sorted.binary 
	$(LOCALE_POSIX); grep -v ":" $*.vocabulary | cut -c1-$(HYB_PREFIX_LENGTH) \
	  | $(SORT) -u > $*.hybrid.prefixes_1
	$(LOCALE_POSIX); perl -ne '/^(c[et]:[^:]+):/ and print "$$1\n";' < $*.vocabulary \
	  | $(SORT) -u > $*.hybrid.prefixes_2
	$(LOCALE_POSIX); perl -ne '/^(C:[^:]+:)/ and print "$$1\n";' < $*.vocabulary \
	  | $(SORT) -u > $*.hybrid.prefixes_3
	$(LOCALE_POSIX); perl -ne '/^(:filter:[^:]+:[^C:\n]{$(HYB_PREFIX_LENGTH)})/ and print "$$1\n";' < $*.vocabulary \
	  | $(SORT) -u > $*.hybrid.prefixes_4
	$(LOCALE_POSIX); perl -ne '/^(:filter:[^:]+:C:[^:]+:)/ and print "$$1\n";' < $*.vocabulary \
	  | $(SORT) -u > $*.hybrid.prefixes_5
	$(LOCALE_POSIX); perl -ne '/^(:facet:[^:]+:)/ and print "$$1\n";' < $*.vocabulary \
	  | $(SORT) -u > $*.hybrid.prefixes_6
	$(LOCALE_POSIX); grep ^:e:entity: $(DB).vocabulary | perl -e 'while(<>) { $$c++; print if $$c % 10000 == 0; }' \
	  | grep -v $(PERSONPATH) | $(SORT) -u > $*.hybrid.prefixes_7
	$(LOCALE_POSIX); grep ^:ec: $(DB).vocabulary | perl -e 'while(<>) { $$c++; print if $$c % 10000 == 0; }' \
	  | $(SORT) -u > $*.hybrid.prefixes_8
	$(LOCALE_POSIX); grep ^:ee:entity: $(DB).vocabulary | perl -e 'while(<>) { $$c++; print if $$c % 10000 == 0; }' \
	  | $(SORT) -u > $*.hybrid.prefixes_9
	$(LOCALE_POSIX); grep $(PERSONPATH) $(DB).vocabulary | perl -e '$$last = ""; while(<>) {@cols = split( /:/, $$_ ); if ($$#cols > 10) { $$#cols = 9; $$path = join(":", @cols); print unless $$path eq $$last; $$last = $$path; }}' \
	  | $(SORT) -u > $*.hybrid.prefixes_10
	$(LOCALE_POSIX); $(SORT) -m $*.hybrid.prefixes_* \
	  | grep -v "^#" \
	  | perl -ne 'chomp; print "$$_\n" unless /[\x00-\x20\x7f-\xff]/;' \
	  > $*.hybrid.prefixes
	rm -f $*.hybrid.prefixes_* 
	$(CS_CODE_DIR)/server/buildIndex HYB $*.words -b $*.hybrid.prefixes -f BINARY #-L
	  2>$*.hybrid.build-index-errors | tee $*.hybrid.build-index-log
	ln -sf $*.hybrid $@
	
%.docs-sorted: %.docs-unsorted
	$(LOCALE_POSIX); $(SORT) -b -k1,1n $*.docs-unsorted > $*.docs-sorted
	
$(DB).words-sorted.binary: $(DB).words-unsorted.binary
	cd $(DATA_DIRECTORY); \
	cp -f $(DBTAIL).words-unsorted.binary $(DBTAIL).words-sorted.binary; \
	$(CS_CODE_DIR)/binarysort/BinarySortMain -u --binary-words-file=$(DBTAIL).words-sorted.binary --words-file-base=$(DBTAIL) --stxxl-path=./
		
$(DB).words-unsorted.binary: $(DB).words-unsorted.ascii
	codebase/binarysort/ConvertAsciiToBinary -b 8192 $^
	
%.words: %.words-unsorted.ascii
	@$(LOCALE_POSIX); \
	$(SORT) -u $*.words-unsorted.ascii | $(SORT) -b -k1,1 -k2,2n -k4,4n > $*.words-sorted.ascii
	
parse-rel: build-semsearch $(YagoFacts) $(YagoPaths) $(YagoRelations) $(RedirectMap) $(WordnetSynonyms) $(ONTOLOGY_ABSTRACTNESS_COUNTS) $(DB).xml
	parser/SemanticWikipediaParserMain $(ParseOptionsWithRelations) $(DB); \
	cat $(DB).words-unsorted.ascii.appendix >> $(DB).words-unsorted.ascii; \
	cat $(DB).docs-unsorted.appendix >> $(DB).docs-unsorted
	
parse-news: build-semsearch $(YagoFacts) $(YagoPaths) $(YagoRelations) $(RedirectMap) $(WordnetSynonyms) $(ONTOLOGY_ABSTRACTNESS_COUNTS) $(DB).xml
	parser/SemanticWikipediaParserMain $(ParseOptionsForNews) $(DB); \
	cat $(DB).words-unsorted.ascii.appendix >> $(DB).words-unsorted.ascii; \
	cat $(DB).docs-unsorted.appendix >> $(DB).docs-unsorted

sw-parse: build-semsearch $(YagoFacts) $(YagoPaths) $(RedirectMap) $(WordnetSynonyms) $(ONTOLOGY_ABSTRACTNESS_COUNTS) $(DB).xml
	parser/SemanticWikipediaParserMain $(ParseOptionsSUSI) $(DB); \
	cat $(DB).words-unsorted.ascii.appendix >> $(DB).words-unsorted.ascii
		
parse-prechunk: build-semsearch $(YagoFacts) $(YagoPaths) $(RedirectMap) $(WordnetSynonyms) $(ONTOLOGY_ABSTRACTNESS_COUNTS) $(DB).xml
	parser/SemanticWikipediaParserMain -c -C -e $(ParseOptionsSUSI) $(DB)

parse-prechunk-rel: build-semsearch $(YagoFacts) $(YagoPaths) $(WordnetSynonyms) $(RedirectMap) $(ONTOLOGY_ABSTRACTNESS_COUNTS) $(DB).xml
	parser/SemanticWikipediaParserMain -c -C -e $(ParseOptionsWithRelations) $(DB)	

parse-pure: build-semsearch $(WordnetSynonyms) $(ONTOLOGY_ABSTRACTNESS_COUNTS)
	parser/SemanticWikipediaParserMain $(DefaultParseOptions) $(DB)
	 

# -----------------------------------------------------------------------------
# Targets for generating input files.
# -----------------------------------------------------------------------------

$(WordnetSynonyms):
	cut -f2,3 $(YAGODIR)/facts/means/WordNetLinks.txt | sed "s/^\"\(.*\)\"/\1/" | sed "s/wordnet_\([^\t]*\)_[0-9]*/\1/" > $@
	#gawk -F'\t' '{print$$2,$$3}' $(YAGODIR)/facts/means/WordNetLinks.txt | sed "s/^\"\(.*\)\"/\1\t/g" | sed "s/wordnet_\([^\t]*\)_[0-9]*/\1/g" > $@
	
$(RedirectMap): $(RedirectMapInput).redirect-map-unsorted
	sort -u $< > $@
	
$(RedirectMapInput).redirect-map-unsorted:
	tools/RedirectMapBuilderMain $(RedirectMapInput)
	
$(DATA_DIRECTORY)/semantic-wikipedia-scientists.xml: $(DATA_DIRECTORY)/semantic-wikipedia.scientist-titles 
	tools/SemanticWikipediaExtractorMain --titles-file $< --input-file $(DATA_DIRECTORY)/$(DBFULL).xml --output-file $@
	
$(DATA_DIRECTORY)/semantic-wikipedia-medicine.xml: $(DATA_DIRECTORY)/semantic-wikipedia-medicine.titles
	tools/SemanticWikipediaExtractorMain --titles-file $< --input-file $(DATA_DIRECTORY)/$(DBFULL).xml --output-file $@

$(DATA_DIRECTORY)/semantic-wikipedia-relations.xml: $(DATA_DIRECTORY)/semantic-wikipedia.rel-titles 
	tools/SemanticWikipediaExtractorMain --titles-file $< --in-text-substring mathemat --input-file $(DATA_DIRECTORY)/$(DBFULL).xml --output-file $@

$(DATA_DIRECTORY)/semantic-wikipedia.scientist-titles:
	grep wordnet_scientist $(ArticleExtractor) $(IsAExtractor) | cut -f2 | sort -u > $@
	
$(DATA_DIRECTORY)/semantic-wikipedia.rel-titles:
	grep "wordnet_mathematician\|wordnet_region" $(ArticleExtractor) $(IsAExtractor) | cut -f2 | sort -u > $@

buildRedirectMap: $(RedirectMap)

# -----------------------------------------------------------------------------
# Targets for generating ontology files
# -----------------------------------------------------------------------------

$(ONTOLOGY)/facts.is-a.entity.class.txt: ontology-manager/get-yago-leaves.pl $(ArticleExtractor) $(WordNetLinks) $(ConceptLinker)
	perl $^ $(ONTOLOGY)/facts.is-a.entity.class.txt 
	
ReplaceHashSymbols =| sed 's/\#/_/g'
FormatDate = | sed 's/\#/0/g' | sed 's/-//g' | gawk -F'\t' '{printf ("%s\t:date:%.8d\n", $$2, $$3)}'
FormatInteger = | cut -f2,3 | sed 's/\t/\t:integer:/g' | sed 's/+//g'
FormatFloat = | cut -f2,3 | sed 's/\t/\t:float:/g' | sed 's/\#/0/g' | sed 's/+//g'  

	
process-yago: 
	rm -f $(ONTOLOGY)/facts.*
	rm -f $(ONTOLOGY)/corrected.*
	rm -f $(ONTOLOGY)/__tmp.*
	rm -f $(ONTOLOGY)/corrections-done.txt
	perl ontology-manager/get-yago-leaves.pl $(ArticleExtractor) $(WordNetLinks) $(ConceptLinker) $(ONTOLOGY)/facts.is-a.Entity.Class.txt
	cut -f2,3 $(YAGODIR)/facts/bornIn/CheckedFactExtractor.txt > $(ONTOLOGY)/facts.born-in.Person.Location.txt
	cut -f2,3 $(YAGODIR)/facts/livesIn/CheckedFactExtractor.txt > $(ONTOLOGY)/facts.lives-in.Person.Location.txt
	cut -f2,3 $(YAGODIR)/facts/diedIn/CheckedFactExtractor.txt > $(ONTOLOGY)/facts.died-in.Person.Location.txt
	cut -f2,3 $(YAGODIR)/facts/locatedIn/CheckedFactExtractor.txt > $(ONTOLOGY)/facts.located-in.Location.Location.txt
	cut -f2,3 $(YAGODIR)/facts/actedIn/CheckedFactExtractor.txt > $(ONTOLOGY)/facts.acted-in.Actor.Movie.txt
	cut -f2,3 $(YAGODIR)/facts/created/CheckedFactExtractor.txt > $(ONTOLOGY)/facts.created.Entity.Entity.txt
	cut -f2,3 $(YAGODIR)/facts/worksAt/CheckedFactExtractor.txt > $(ONTOLOGY)/facts.works-at.Person.Organization.txt
	cut -f2,3 $(YAGODIR)/facts/graduatedFrom/CheckedFactExtractor.txt > $(ONTOLOGY)/facts.graduated-from.Person.University.txt
	cut -f2,3 $(YAGODIR)/facts/hasWonPrize/CheckedFactExtractor.txt > $(ONTOLOGY)/facts.has-won-prize.Person.Award.txt
	cut -f2,3 $(YAGODIR)/facts/dealsWith/CheckedFactExtractor.txt > $(ONTOLOGY)/facts.deals-with.Country.Country.txt
	cut -f2,3 $(YAGODIR)/facts/directed/CheckedFactExtractor.txt > $(ONTOLOGY)/facts.directed.Director.Movie.txt
	cut -f2,3 $(YAGODIR)/facts/discovered/CheckedFactExtractor.txt > $(ONTOLOGY)/facts.discovered.Person.Entity.txt
	cut -f2,3 $(YAGODIR)/facts/isCitizenOf/CheckedFactExtractor.txt > $(ONTOLOGY)/facts.is-citizen-of.Person.Country.txt
	cut -f2,3 $(YAGODIR)/facts/hasAcademicAdvisor/CheckedFactExtractor.txt > $(ONTOLOGY)/facts.has-academic-advisor.Person.Person.txt
	cut -f2,3 $(YAGODIR)/facts/politicianOf/CheckedFactExtractor.txt > $(ONTOLOGY)/facts.politician-of.Person.Location.txt
	cut -f2,3 $(YAGODIR)/facts/exports/ArticleExtractor.txt > $(ONTOLOGY)/facts.exports.Country.Entity.txt
	cut -f2,3 $(YAGODIR)/facts/hasCapital/CheckedFactExtractor.txt > $(ONTOLOGY)/facts.has-capital.Location.Location.txt
	cut -f2,3 $(YAGODIR)/facts/hasOfficialLanguage/CheckedFactExtractor.txt > $(ONTOLOGY)/facts.has-official-language.Country.Language.txt
	cat $(YAGODIR)/facts/bornOnDate/ArticleExtractor.txt $(FormatDate) > $(ONTOLOGY)/facts.born-on-date.Person.value_date.txt
	cat $(YAGODIR)/facts/diedOnDate/ArticleExtractor.txt $(FormatDate) > $(ONTOLOGY)/facts.died-on-date.Person.value_date.txt
	cat $(YAGODIR)/facts/hasPopulation/ArticleExtractor.txt $(FormatFloat) > $(ONTOLOGY)/facts.has-population.Location.value_float.txt
	cat $(YAGODIR)/facts/hasPopulationDensity/ArticleExtractor.txt $(FormatFloat) | sed 's/\/km^2//' > $(ONTOLOGY)/facts.has-population-density-per-km^2.Location.value_float.txt
	
	
susi-input: build-semsearch $(ONTOLOGY)/facts.is-a.entity.class.txt $(ONTOLOGY)/corrections.txt
	ontology-manager/OntologyManagerMain --redirect-map $(RedirectMap) --susi-input $(DATA_DIRECTORY) $(ONTOLOGY)  
	
parser-input: build-semsearch $(DATA_DIRECTORY)/wordnet-disambiguation-map $(WordnetSynonyms)
	ontology-manager/OntologyManagerMain --redirect-map $(RedirectMap) --susi-input $(DATA_DIRECTORY) --relations $(DATA_DIRECTORY)/semantic-wikipedia.yago-relations -w $(DATA_DIRECTORY)/wordnet-disambiguation-map  -W $(WordnetSynonyms) $(ONTOLOGY)
	
parser-input-old: build-semsearch
	ontology-manager/OntologyManagerMain --susi-input $(DATA_DIRECTORY) --relations $(DATA_DIRECTORY)/semantic-wikipedia.yago-relations $(ONTOLOGY)	

# -----------------------------------------------------------------------------
# Targets specific to the evaluation of the SemanticWikipedia Search.
# -----------------------------------------------------------------------------

# Queries:
CompSciQuery = "computer :e:entity:physicalentity:object:whole:livingthing:organism:person:scientist:*"
CompSciQuery2 = "computer science :e:entity:physicalentity:object:whole:livingthing:organism:person:scientist:*"
CompSciQuery3 = "computer scientist :e:entity:physicalentity:object:whole:livingthing:organism:person:scientist:*"
TreatyQuery = ":e:entity:abstraction:communication:message:statement:agreement:writtenagreement:treaty:*"
TreatyQuery2 = "treaty :e:entity:abstraction:communication:message:statement:agreement:writtenagreement:treaty:*"
FranceRegQuery= "france :e:entity:physicalentity:object:location:region:*"
FranceRegQuery2 = "region of france :e:entity:physicalentity:object:location:region:*"
CocktailsQuery = ":e:entity:physicalentity:matter:substance:food:beverage:alcohol:mixeddrink:cocktail:*"
PolAutQuery = "political :e:entity:physicalentity:object:whole:livingthing:organism:person:communicator:writer:*"
EmiArtQuery = "emi :e:entity:physicalentity:object:whole:livingthing:organism:person:creator:artist:*"
EmiArtQuery2 = "emi :e:entity:physicalentity:object:whole:livingthing:organism:person:entertainer:performer:musician:*"
EngMonQuery = "english :e:entity:physicalentity:object:whole:livingthing:organism:person:ruler:sovereign:*"
SatFatAcidQuery = "saturated fatty :e:entity:physicalentity:matter:substance:material:chemical:compound:acid:*"
DrugDeathQuery = "drug death :e:entity:physicalentity:object:whole:livingthing:organism:person:*"
DrugDeathQuery2 = "drug died|death :e:entity:physicalentity:object:whole:livingthing:organism:person:*"
DrugDeathQuery3 = ":e:entity:physicalentity:matter:substance:agent:drug:*|drug died|death :e:entity:physicalentity:object:whole:livingthing:organism:person:*"
PresidentQuery = "united states :e:entity:physicalentity:object:whole:livingthing:organism:person:leader:head:administrator:executive:corporateexecutive:president:*"
PresidentQuery2 = "united states elected :e:entity:physicalentity:object:whole:livingthing:organism:person:leader:head:administrator:executive:corporateexecutive:president:*"

#Settings for top-k retrieval
TOPK = 100000

eval: 
	echo "Doing quality evaluation now:\n";
	#$(MAKE) eval-quality
	echo "Doing performance evaluation now:\n";
	$(CS_CODE_DIR)/server/answerQueries HYB -h $(MAX_SIZE_HISTORY) $(DB).hybrid -q $(EVALBASE)/semantic-wikipedia-evaluation.performance-queries -s $(EVALBASE)/semantic-wikipedia.performance-statistics | grep millisec > $(EVALBASE)/semantic-wikipedia.performance-statistics 

eval-quality: build-semsearch
	rm -f $(EVALBASE)/semantic-wikipedia-evaluation.ground-truth;
	$(MAKE) $(EVALBASE)/semantic-wikipedia-evaluation.ground-truth; 
	tools/evaluation/SemanticWikipediaEvaluationMain --top-k $(TOPK) --server-port $(PORT) --host localhost --redirect-map $(RedirectMap) $(EVALBASE)/semantic-wikipedia-evaluation.ground-truth $(EVALBASE)/semantic-wikipedia-evaluation.result
	perl $(EVALBASE)/result-statistics.pl $(EVALBASE)/semantic-wikipedia-evaluation.result  > $(EVALBASE)/semantic-wikipedia-evaluation.result-statistics
	
$(EVALBASE)/semantic-wikipedia-evaluation.ground-truth: $(EVALBASE)/semantic-wikipedia-evaluation-lists.xml
	touch $@;
	perl $(EVALBASE)/parse-list-of-computer-scientists.pl $< $(CompSciQuery) $@;
	perl $(EVALBASE)/parse-list-of-computer-scientists.pl $< $(CompSciQuery2) $@
	perl $(EVALBASE)/parse-list-of-treaties.pl $< $(TreatyQuery) $@;
	perl $(EVALBASE)/parse-list-of-treaties.pl $< $(TreatyQuery2) $@;
	perl $(EVALBASE)/parse-regions-of-france.pl $< $(FranceRegQuery) $@;
	perl $(EVALBASE)/parse-regions-of-france.pl $< $(FranceRegQuery2) $@;
	perl $(EVALBASE)/parse-list-of-cocktails.pl $< $(CocktailsQuery) $@;
	perl $(EVALBASE)/parse-list-of-political-authors.pl $< $(PolAutQuery) $@;
	perl $(EVALBASE)/parse-list-of-EMI-artists.pl $< $(EmiArtQuery) $@;
	perl $(EVALBASE)/parse-list-of-EMI-artists.pl $< $(EmiArtQuery2) $@;
	perl $(EVALBASE)/parse-list-of-english-monarchs.pl $< $(EngMonQuery) $@;
	perl $(EVALBASE)/parse-list-of-saturated-fatty-acids.pl $< $(SatFatAcidQuery) $@;
	perl $(EVALBASE)/parse-list-of-drug-related-deaths.pl $< $(DrugDeathQuery) $@;
	perl $(EVALBASE)/parse-list-of-drug-related-deaths.pl $< $(DrugDeathQuery2) $@;
	perl $(EVALBASE)/parse-list-of-drug-related-deaths.pl $< $(DrugDeathQuery3) $@;
	perl $(EVALBASE)/parse-list-of-presidents-of-the-united-states.pl $< $(PresidentQuery) $@;
	perl $(EVALBASE)/parse-list-of-presidents-of-the-united-states.pl $< $(PresidentQuery2) $@;
	
$(EVALBASE)/semantic-wikipedia-evaluation-lists.xml: $(EVALBASE)/semantic-wikipedia-evaluation-lists.titles
	tools/SemanticWikipediaExtractorMain --titles-file $< --input-file $(DATA_DIRECTORY)/$(DBFULL).xml --output-file $@


