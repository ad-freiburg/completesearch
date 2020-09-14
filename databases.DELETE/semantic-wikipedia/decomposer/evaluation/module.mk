MDIR = evaluation
# SRC += $(filter-out %Main.cpp %Test.cpp, $(wildcard $(MDIR)/*.cpp))
SRC += evaluation/GroundTruthReader.cpp
#MAINFILES += $(filter %Main.cpp, $(wildcard $(MDIR)/*.cpp))
#TESTFILES += $(filter %Test.cpp, $(wildcard $(MDIR)/*.cpp))


# -----------------------------------------------------------------------------
# Targets specific to running the evaluation of decomposers.
# -----------------------------------------------------------------------------

eval-ml-decomp: evaluation/ContextDecomposerEvaluationMain
	rm -f $(CONTEXTGT).ml.result
	evaluation/ContextDecomposerEvaluationMain -v -e $(CONTEXTEQUALITY_POSTAGS) -g $(GPOSTTL_DIR)/data  -m $(MAP_FILE) -l $(CLAUSEMAP_FILE) -f $(FEATURECONFIG_FILE) -c $(MODEL_DIR)/lib_rel,"REL",$(MODEL_DIR)/lib_lit,"LIT",$(MODEL_DIR)/lib_rela,"RELA" -w $(MODEL_DIR)/lib_sep,"SEP" -k $(MODEL_DIR)/lib_rel_close,"REL)",$(MODEL_DIR)/lib_rela_close,"RELA)",$(MODEL_DIR)/lib_lit_close,"LIT)" -o $(MODEL_DIR)/lib_rel_open,"REL(",$(MODEL_DIR)/lib_rela_open,"RELA(",$(MODEL_DIR)/lib_lit_open,"LIT(" -d ml $(CONTEXTGT) $(CONTEXTGT).ml.result

eval-ml-decomp-test:
	$(MAKE) eval-ml-decomp GT=testsentences.txt

eval-rule-decomp: evaluation/ContextDecomposerEvaluationMain
	rm -f $(CONTEXTGT).rule.result
	evaluation/ContextDecomposerEvaluationMain -v -e $(CONTEXTEQUALITY_POSTAGS) -g $(GPOSTTL_DIR)/data  -d robust $(CONTEXTGT) $(CONTEXTGT).rule.result

eval-rule-decomp-test:
	$(MAKE) eval-rule-decomp GT=testsentences.txt

eval-marker: evaluation/ContextMarkerEvaluationMain
	rm -f $(CONTEXTGT).marker_result
	evaluation/ContextMarkerEvaluationMain -v -m $(MAP_FILE) -l $(CLAUSEMAP_FILE) -f $(FEATURECONFIG_FILE) -c $(MODEL_DIR)/lib_rel,"REL",$(MODEL_DIR)/lib_lit,"LIT",$(MODEL_DIR)/lib_rela,"RELA" -w $(MODEL_DIR)/lib_sep,"SEP" -k $(MODEL_DIR)/lib_rel_close,"REL)",$(MODEL_DIR)/lib_rela_close,"RELA)",$(MODEL_DIR)/lib_lit_close,"LIT)" -o $(MODEL_DIR)/lib_rel_open,"REL(",$(MODEL_DIR)/lib_rela_open,"RELA(",$(MODEL_DIR)/lib_lit_open,"LIT(" $(CONTEXTGT) $(CONTEXTGT).marker_result

eval-marker-test:
	$(MAKE) eval-marker	GT=testsentences.txt

eval-alldecomp:
	$(MAKE) eval-rule-decomp
	$(MAKE) eval-ml-decomp

eval-alldecomp-test:
	$(MAKE) eval-rule-decomp GT=testsentences.txt
	$(MAKE) eval-ml-decomp GT=testsentences.txt


add-gt:
	util/tagchunk-sentence.py "$(SENT)" | $(EVAL_DIR)/add-ground-truth.pl $(CONTEXTGT)
