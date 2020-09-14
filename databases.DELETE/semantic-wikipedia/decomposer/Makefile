# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------
# New version of this Makefile 17 of March 2011 by Elmar
# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------

include ../Makefile

CXXFLAGS = -O6 -Wall -Wno-deprecated -DLOGLEVEL=$(LOGLEVEL)
#CXXFLAGS = -O0 -pg -g -Wall -Wno-deprecated -DLOGLEVEL=$(LOGLEVEL)
LOGLEVEL = 3
CXXINCLUDES = -I$(GTEST_DIR)/include -I$(SEM_WIK_DIR)/decomposer
CXX = g++ $(CXXFLAGS) $(CXXINCLUDES)

# Paths and files
UTILS_DIR = $(CS_CODE_DIR)/semantic-wikipedia-utils
DECOMP_DIR = $(SEM_WIK_DIR)/decomposer
GPOSTTL_DIR = /home/haussmae/tagger/gposttl/gposttl-0.9.3
SVMLIGHT_DIR = /home/haussmae/util/svm-light
LIBSVM_DIR = /home/haussmae/util/libsvm/libsvm-3.1
SIDDHARTHANSIMPLIFY = /home/haussmae/systems/simplify/simplify-sdp_wrap.sh
INDEXTAGGER = /home/haussmae/tagger/tree-tagger/cmd/tree-tagger-english-wiki-newpenn
INDEXCHUNKER = /home/haussmae/tagger/yamcha/yamcha_wrap
INDEXTAGGER_DEEP = /home/haussmae/tagger/stanford-postagger-2011-09-14/WordsUnsortedTagger.sh
INDEXPARSER = /home/haussmae/parser/chunkparser-1.1/parser.sh
INDEXSENNA = "/home/haussmae/systems/senna/senna-v3.0/senna -pos -psg "
CONFIG_DIR = $(DECOMP_DIR)/config
FEATURECONFIG_FILE = $(CONFIG_DIR)/default_feature_config.txt
STOPWORDS_FILE = $(SEM_WIK_DIR)/semantic-wikipedia.stop-words
CLITICS_FILE = $(CONFIG_DIR)/semantic-wikipedia-decomposer.clitics
GT = context-decomposer-ground-truths.txt
TESTGT = testsentences.txt
EVAL_DIR = $(DECOMP_DIR)/evaluation
MODEL_DIR = $(DECOMP_DIR)/decomposer-ml/models
FEATURES_DIR = $(DECOMP_DIR)/decomposer-ml/features
CONTEXTGT = $(EVAL_DIR)/$(GT)
FEATURES_SOURCE_FILE = $(CONTEXTGT)
FEATURE_FILE_PREFIX = $(notdir $(FEATURES_SOURCE_FILE))
FEATURE_FILE_FULLPREFIX = $(FEATURES_DIR)/$(notdir $(FEATURES_SOURCE_FILE))
MAP_FILE = $(FEATURE_FILE_FULLPREFIX).feature.map
CLAUSEMAP_FILE = $(FEATURE_FILE_FULLPREFIX).clause-feature.map

# Commands
SVMLIGHTLEARN = $(SVMLIGHT_DIR)/svm_learn -t 2 -b 0
SVMLIBLEARN = $(LIBSVM_DIR)/tools/easy.py
SVMLIBLEARNLIN = $(LIBSVM_DIR)/svm-train -t 0
SVMLIBPREDICT = $(LIBSVM_DIR)/svm-predict



# Object files used for compilations
GPOSTTL_OBJFILES = $(GPOSTTL_DIR)/src/tagger_controller.o $(GPOSTTL_DIR)/src/darray.o $(GPOSTTL_DIR)/src/enhance_penntag.o  $(GPOSTTL_DIR)/src/final-state-tagger.o  $(GPOSTTL_DIR)/src/lex.o  $(GPOSTTL_DIR)/src/memory.o  $(GPOSTTL_DIR)/src/registry.o  $(GPOSTTL_DIR)/src/start-state-tagger.o $(GPOSTTL_DIR)/src/tagger.o  $(GPOSTTL_DIR)/src/tokenizer.o  $(GPOSTTL_DIR)/src/useful.o
SVMLIGHT_OBJFILES = $(SVMLIGHT_DIR)/svm_learn.o $(SVMLIGHT_DIR)/svm_common.o $(SVMLIGHT_DIR)/svm_hideo.o
LIBSVM_OBJFILES = $(LIBSVM_DIR)/svm.o

# Parameters
MINWORDLENGTH = 2
PHRASEONLY = 1
LOCKFMAP = 0
CONTEXTEQUALITY_POSTAGS = "PP$$ NNP NNS NP NN VBD VBZ VBP JJ VB RB PP PRP VBN CD"
# For full-sep11 this gives 29 split-files
SPLITLINES = 100000
#SPLITLINES = 1000000
PARALLEL = 10

# Default settings: use rule based decomposer and no parallel processing
DECOMPOSER = rule
SPLIT = no

ifeq ($(DECOMPOSER),rule)
	TAGGER = $(INDEXTAGGER)
	CHUNKER = $(INDEXCHUNKER)
endif
ifeq ($(DECOMPOSER),deep)
#  	TAGGER = $(INDEXTAGGER_DEEP)
#	CHUNKER = $(INDEXPARSER)
  	TAGGER = NONE
	CHUNKER = $(INDEXSENNA)
endif

# -----------------------------------------------------------------------------
# Targets specific to running the decomposers.
# -----------------------------------------------------------------------------

$(DB).words-unsorted.prechunk.tagged.%: $(DB).words-unsorted.prechunk.%
	$(DECOMP_DIR)/util/tagchunk-index.py $< $(TAGGER) $(CHUNKER) $@

index-append: $(DB).words-unsorted.prechunk.ascii.appendix $(DB).docs-unsorted.appendix $(DB).words-unsorted.ascii
	$(DECOMP_DIR)/util/append-index.py $(DB).words-unsorted.ascii \
		$(DB).words-unsorted.prechunk.ascii.appendix \
		$(DB).docs-unsorted $(DB).docs-unsorted.appendix

decompose-index-rule:  $(DB).words-unsorted.prechunk.tagged.ascii base/SemanticWikipediaDecomposerMain
	$(DECOMP_DIR)/base/SemanticWikipediaDecomposerMain $(DB) --decomposer $(DECOMPOSER) --stopWordsFile $(STOPWORDS_FILE) --cliticsFile $(CLITICS_FILE) --minimumWordLength $(MINWORDLENGTH)
	$(MAKE) index-append

decompose-file-rule:  base/SemanticWikipediaDecomposerMain 
	$(DECOMP_DIR)/base/SemanticWikipediaDecomposerMain $(DB) --decomposer $(DECOMPOSER) --wordsUnsortedFile $(WORDS_UNSORTED_FILE) --stopWordsFile $(STOPWORDS_FILE) --cliticsFile $(CLITICS_FILE) --minimumWordLength $(MINWORDLENGTH)

decompose-index-rule-split:  base/SemanticWikipediaDecomposerMain
	rm -f $(DB).words-unsorted.ascii
	rm -f $(DB).docs-unsorted
	rm -f $(DB)*split*
	rm -f $(DB).task
	split -a 3 -d -l $(SPLITLINES) $(DB).words-unsorted.prechunk.ascii $(DB).words-unsorted.prechunk.ascii.split
	util/create-tag-tasks.py $(DB)
	cat $(DB).task | xargs -n1 -P$(PARALLEL) make
	rm -f $(DB).task
	util/create-decompose-tasks.py $(DB)
	cat $(DB).task | xargs -n1 -P$(PARALLEL) make decompose-file-rule
	rm -f $(DB).task
	util/create-append-tasks.py $(DB)
	cat $(DB).task | xargs -n4 -P1 $(DECOMP_DIR)/util/append-index.py
	rm -f $(DB).task
	$(MAKE) index-append
	#rm -f $(DB).words-unsorted.prechunk.tagged.ascii.split*
	#rm -f $(DB).words-unsorted.prechunk.tagged.ascii.split*
	#rm -f $(DB).words-unsorted.prechunk.ascii.split*
	#rm -f $(DB).words-unsorted.prechunk.ascii.split*
	
decompose-index-ml:   $(DB).words-unsorted.prechunk.tagged.ascii base/SemanticWikipediaDecomposerMain
	$(DECOMP_DIR)/base/SemanticWikipediaDecomposerMain -m $(MAP_FILE) -l $(CLAUSEMAP_FILE) -f $(FEATURECONFIG_FILE) \
		--stopWordsFile $(STOPWORDS_FILE) \
    	-c $(MODEL_DIR)/lib_rel,"REL",$(MODEL_DIR)/lib_lit,"LIT",$(MODEL_DIR)/lib_rela,"RELA" \
    	-w $(MODEL_DIR)/lib_sep,"SEP" \
    	-k $(MODEL_DIR)/lib_rel_close,"REL)",$(MODEL_DIR)/lib_rela_close,"RELA)",$(MODEL_DIR)/lib_lit_close,"LIT)" \
    	-o $(MODEL_DIR)/lib_rel_open,"REL(",$(MODEL_DIR)/lib_rela_open,"RELA(",$(MODEL_DIR)/lib_lit_open,"LIT(" \
    	-d ml \
	 $(DB)
	 $(MAKE) index-append
	 
	 
decompose-index-ml-suffix:   SemanticWikipediaDecomposerMain
	$(DECOMP_DIR)/base/SemanticWikipediaDecomposerMain -m $(MAP_FILE) -l $(CLAUSEMAP_FILE) -f $(FEATURECONFIG_FILE) \
    	-c $(MODEL_DIR)/lib_rel,"REL",$(MODEL_DIR)/lib_lit,"LIT",$(MODEL_DIR)/lib_rela,"RELA" \
    	-w $(MODEL_DIR)/lib_sep,"SEP" \
    	-k $(MODEL_DIR)/lib_rel_close,"REL)",$(MODEL_DIR)/lib_rela_close,"RELA)",$(MODEL_DIR)/lib_lit_close,"LIT)" \
    	-o $(MODEL_DIR)/lib_rel_open,"REL(",$(MODEL_DIR)/lib_rela_open,"RELA(",$(MODEL_DIR)/lib_lit_open,"LIT(" \
    	-d ml \
	 $(DB)

clean-decompose:
	rm -f $(DB).words-unsorted.prechunk.ascii.tagged*
	rm -f $(DB).words-unsorted.prechunk.tagged*
	rm -f $(DB).words-unsorted.ascii

.PHONY = all-swd clean-swd depend-all-swd test-swd

MODULES := sentence util base decomposer-ml decomposer-rule decomposer-rule-deep evaluation

#CXXFLAGS += $(patsubst %,-I ./%, $(MODULES))
#CXXFLAGS += -DLOGLEVEL=4

# extra libs
LIBS := -lboost_regex-mt
TEST_LIBS := -lboost_regex-mt -lexpat

# each module adds to these
SRC :=
TESTFILES :=
MAINFILES :=

# include modules
include $(patsubst %, %/module.mk,$(MODULES))

# determine all obj files
OBJS := $(patsubst %.cpp,%.o, $(filter %.cpp,$(SRC)))

# additional 3-rd party object files needed for linking
LINK_OBJS = $(LIBSVM_OBJFILES) $(SVMLIGHT_OBJFILES) $(GPOSTTL_OBJFILES)

base/SemanticWikipediaDecomposerMain: base/SemanticWikipediaDecomposerMain.o $(OBJS)
	$(CXX) -o $@ $(LINK_OBJS) $^ $(LIBS)

decomposer-ml/GroundTruthClassifierMain: decomposer-ml/GroundTruthClassifierMain.o $(OBJS)
	$(CXX) -o $@ $(LINK_OBJS) $^ $(LIBS)

decomposer-ml/GroundTruthClauseFeatureExtractorMain: decomposer-ml/GroundTruthClauseFeatureExtractorMain.o $(OBJS)
	$(CXX) -o $@ $(LINK_OBJS) $^ $(LIBS)

decomposer-ml/GroundTruthFeatureExtractorMain: decomposer-ml/GroundTruthFeatureExtractorMain.o $(OBJS)
	$(CXX) -o $@ $(LINK_OBJS) $^ $(LIBS)
	
decomposer-ml/SVMLightClassifierMain: decomposer-ml/SVMLightClassifierMain.o $(OBJS)
	$(CXX) -o $@ $(LINK_OBJS) $^ $(LIBS)

decomposer-ml/SVMStructClassifierInputMain: decomposer-ml/SVMStructClassifierInputMain.o $(OBJS)
	$(CXX) -o $@ $(LINK_OBJS) $^ $(LIBS)

evaluation/ContextDecomposerEvaluationMain: evaluation/ContextDecomposerEvaluationMain.o $(OBJS)
	$(CXX) -o $@ $(LINK_OBJS) $^ $(LIBS)

evaluation/ContextMarkerEvaluationMain: evaluation/ContextMarkerEvaluationMain.o $(OBJS)
	$(CXX) -o $@ $(LINK_OBJS) $^ $(LIBS)
	
decomposer-rule-deep/DeepContextMarkerConsoleMain: decomposer-rule-deep/DeepContextMarkerConsoleMain.o $(OBJS)
	$(CXX) -o $@ $(LINK_OBJS) $^ $(LIBS)


%Test: %Test.cpp $(OBJS)
	$(CXX)  -o $@ $^ $(LINK_OBJS) $(TEST_LIBS) $(TEST_INCLUDE)

%.o: %.cpp
	$(CXX) -c -o $@ $<

# calculate dependencies
%.d: %.cpp
	./depend.sh `dirname $*.cpp` $(CXXFLAGS) $*.cpp > $@

all-swd: $(MAINFILES:.cpp=) $(TESTFILES:.cpp=)

depend-all-swd: $(OBJS:.o=.d)

test-swd: $(TESTFILES:.cpp=)
	$(foreach i, $(TESTFILES:.cpp=), ./$(i);)

lint-swd:
	$(foreach m, $(MODULES), python $(CS_CODE_DIR)/utility/cpplint.py $(m)/*.h $(m)/*.cpp;)

clean-swd:
	$(foreach i, $(MAINFILES:.cpp=), rm -f $(i);)
	$(foreach i, $(TESTFILES:.cpp=), rm -f $(i);)
	$(foreach m, $(MODULES), rm -f $(m)/*.o;)
	$(foreach m, $(MODULES), rm -f $(m)/*.d;)

# include dependencies
ifneq ($(MAKECMDGOALS), clean-swd)
-include $(OBJS:.o=.d)
endif
