MDIR = decomposer-ml
SRC += $(filter-out %Main.cpp %Test.cpp, $(wildcard $(MDIR)/*.cpp))
MAINFILES += $(filter %Main.cpp, $(wildcard $(MDIR)/*.cpp))
TESTFILES += $(filter %Test.cpp, $(wildcard $(MDIR)/*.cpp))


decomposer-ml/LibSVMClassifier.o: decomposer-ml/LibSVMClassifier.cpp
	$(CXX) -I$(LIBSVM_DIR) -o $@ -c $<

decomposer-ml/LibSVMClassifier.d: decomposer-ml/LibSVMClassifier.cpp
	./depend.sh `dirname $<` $(CXXFLAGS) -I$(LIBSVM_DIR) $< > $@

decomposer-ml/SVMLightClassifier.o: decomposer-ml/SVMLightClassifier.cpp
	$(CXX) -I$(SVMLIGHT_DIR) -o $@ -c $<

decomposer-ml/SVMLightClassifier.d: decomposer-ml/SVMLightClassifier.cpp
	./depend.sh `dirname $<` $(CXXFLAGS) -I$(SVMLIGHT_DIR) $< > $@



# -----------------------------------------------------------------------------
# Targets specific to machine learning tasks
# -----------------------------------------------------------------------------

# Extract features from ground truth.
extract-only-features: decomposer-ml/GroundTruthFeatureExtractorMain decomposer-ml/GroundTruthClauseFeatureExtractorMain
	decomposer-ml//GroundTruthFeatureExtractorMain -f $(FEATURECONFIG_FILE) -p $(PHRASEONLY) -b "SEP" -t "" -m $(MAP_FILE) -l $(LOCKFMAP) $(FEATURES_SOURCE_FILE) $(FEATURE_FILE_FULLPREFIX)-sep 
	decomposer-ml//GroundTruthFeatureExtractorMain -f $(FEATURECONFIG_FILE) -p $(PHRASEONLY) -b "REL(" -t "SEP" -m $(MAP_FILE) -l $(LOCKFMAP) $(FEATURES_SOURCE_FILE) $(FEATURE_FILE_FULLPREFIX)-rel_open
	decomposer-ml//GroundTruthFeatureExtractorMain -f $(FEATURECONFIG_FILE) -p $(PHRASEONLY) -b "REL)" -t "SEP REL(" -m $(MAP_FILE) -l $(LOCKFMAP) $(FEATURES_SOURCE_FILE) $(FEATURE_FILE_FULLPREFIX)-rel_close
	decomposer-ml/GroundTruthFeatureExtractorMain -f $(FEATURECONFIG_FILE) -p $(PHRASEONLY) -b "RELA(" -t "SEP REL( REL)" -m $(MAP_FILE) -l $(LOCKFMAP) $(FEATURES_SOURCE_FILE) $(FEATURE_FILE_FULLPREFIX)-rela_open
	decomposer-ml/GroundTruthFeatureExtractorMain -f $(FEATURECONFIG_FILE) -p $(PHRASEONLY) -b "RELA)" -t "SEP REL( REL) RELA(" -m $(MAP_FILE) -l $(LOCKFMAP) $(FEATURES_SOURCE_FILE) $(FEATURE_FILE_FULLPREFIX)-rela_close
	decomposer-ml/GroundTruthFeatureExtractorMain -f $(FEATURECONFIG_FILE) -p $(PHRASEONLY) -b "LIT(" -t "SEP REL( REL) RELA( RELA)" -m $(MAP_FILE) -l $(LOCKFMAP) $(FEATURES_SOURCE_FILE) $(FEATURE_FILE_FULLPREFIX)-lit_open
	decomposer-ml/GroundTruthFeatureExtractorMain -f $(FEATURECONFIG_FILE) -p $(PHRASEONLY) -b "LIT)" -t "SEP REL( REL) RELA( RELA) LIT(" -m $(MAP_FILE) -l $(LOCKFMAP) $(FEATURES_SOURCE_FILE) $(FEATURE_FILE_FULLPREFIX)-lit_close
	decomposer-ml/GroundTruthClauseFeatureExtractorMain -c "LIT" --fmap $(CLAUSEMAP_FILE) --feature_conf $(FEATURECONFIG_FILE) --lockFmap $(LOCKFMAP) $(FEATURES_SOURCE_FILE) $(FEATURE_FILE_FULLPREFIX)-lit
	decomposer-ml/GroundTruthClauseFeatureExtractorMain -c "REL" --fmap $(CLAUSEMAP_FILE) --feature_conf $(FEATURECONFIG_FILE) --lockFmap $(LOCKFMAP) $(FEATURES_SOURCE_FILE) $(FEATURE_FILE_FULLPREFIX)-rel
	decomposer-ml/GroundTruthClauseFeatureExtractorMain -c "RELA" --fmap $(CLAUSEMAP_FILE) --feature_conf $(FEATURECONFIG_FILE) --lockFmap $(LOCKFMAP) $(FEATURES_SOURCE_FILE) $(FEATURE_FILE_FULLPREFIX)-rela

# Extract features but clean up before.
extract-features: 
	rm -f $(MAP_FILE) $(CLAUSEMAP_FILE)
	rm -f $(FEATURE_FILE_FULLPREFIX)-sep.features $(FEATURE_FILE_FULLPREFIX)-rel_open.features $(FEATURE_FILE_FULLPREFIX)-rel_close.features \
		$(FEATURE_FILE_FULLPREFIX)-rela_open.features $(FEATURE_FILE_FULLPREFIX)-rela_close.features $(FEATURE_FILE_FULLPREFIX)-lit_open.features  \
		$(FEATURE_FILE_FULLPREFIX)-lit_close.features $(FEATURE_FILE_FULLPREFIX)-lit $(FEATURE_FILE_FULLPREFIX)-lit_close.features $(FEATURE_FILE_FULLPREFIX)-rel \
		$(FEATURE_FILE_FULLPREFIX)-lit_close.features $(FEATURE_FILE_FULLPREFIX)-rela
	$(MAKE) extract-only-features
	
# Train the SVMLight classifiers on the ground truth. Cleans up, extracts features and trains models.
train-classifier-light: extract-features
	rm -f $(MODEL_DIR)/light_sep $(MODEL_DIR)/light_rel_open $(MODEL_DIR)/light_rel_close $(MODEL_DIR)/light_rela_open $(MODEL_DIR)/light_rela_close $(MODEL_DIR)/light_lit_open $(MODEL_DIR)/light_lit_close
	$(SVMLIGHTLEARN) $(FEATURE_FILE_FULLPREFIX)-sep.features $(MODEL_DIR)/light_sep
	$(SVMLIGHTLEARN) $(FEATURE_FILE_FULLPREFIX)-rel_open.features $(MODEL_DIR)/light_rel_open
	$(SVMLIGHTLEARN) $(FEATURE_FILE_FULLPREFIX)-rel_close.features $(MODEL_DIR)/light_rel_close
	$(SVMLIGHTLEARN) $(FEATURE_FILE_FULLPREFIX)-rela_open.features $(MODEL_DIR)/light_rela_open
	$(SVMLIGHTLEARN) $(FEATURE_FILE_FULLPREFIX)-rela_close.features $(MODEL_DIR)/light_rela_close
	$(SVMLIGHTLEARN) $(FEATURE_FILE_FULLPREFIX)-lit_open.features $(MODEL_DIR)/light_lit_open
	$(SVMLIGHTLEARN) $(FEATURE_FILE_FULLPREFIX)-lit_close.features $(MODEL_DIR)/light_lit_close

# Train the LIBSVM classifiers on the ground truth. Cleans up, extracts features and trains models.
train-classifier-lib: extract-features
	rm -f $(MODEL_DIR)/lib_sep $(MODEL_DIR)/lib_rel_open $(MODEL_DIR)/lib_rel_close $(MODEL_DIR)/lib_rela_open $(MODEL_DIR)/lib_rela_close $(MODEL_DIR)/lib_lit_open $(MODEL_DIR)/lib_lit_close
	cd $(LIBSVM_DIR)/tools; $(SVMLIBLEARN) $(FEATURE_FILE_FULLPREFIX)-sep.features 
	cd $(LIBSVM_DIR)/tools; $(SVMLIBLEARN) $(FEATURE_FILE_FULLPREFIX)-rel_open.features 
	cd $(LIBSVM_DIR)/tools; $(SVMLIBLEARN) $(FEATURE_FILE_FULLPREFIX)-rel_close.features
	cd $(LIBSVM_DIR)/tools; $(SVMLIBLEARN) $(FEATURE_FILE_FULLPREFIX)-rela_open.features
	cd $(LIBSVM_DIR)/tools; $(SVMLIBLEARN) $(FEATURE_FILE_FULLPREFIX)-rela_close.features
	cd $(LIBSVM_DIR)/tools; $(SVMLIBLEARN) $(FEATURE_FILE_FULLPREFIX)-lit_open.features
	cd $(LIBSVM_DIR)/tools; $(SVMLIBLEARN) $(FEATURE_FILE_FULLPREFIX)-lit_close.features
	cd $(LIBSVM_DIR)/tools; $(SVMLIBLEARN) $(FEATURE_FILE_FULLPREFIX)-lit.features
	cd $(LIBSVM_DIR)/tools; $(SVMLIBLEARN) $(FEATURE_FILE_FULLPREFIX)-rel.features
	cd $(LIBSVM_DIR)/tools; $(SVMLIBLEARN) $(FEATURE_FILE_FULLPREFIX)-rela.features
	#$(LIBSVM_DIR)/svm-train -b 1 -t 2 -g 0.0078125 -c 8 $(FEATURE_FILE_FULLPREFIX)-lit.features
	#$(LIBSVM_DIR)/svm-train -b 1 -t 2 -g 0.0078125 -c 0.03125 $(FEATURE_FILE_FULLPREFIX)-rel.features
	#$(LIBSVM_DIR)/svm-train -b 1 -t 2 -g 0.0001220703125 -c 512 $(FEATURE_FILE_FULLPREFIX)-rela.features
	mv -f $(LIBSVM_DIR)/tools/$(FEATURE_FILE_PREFIX)-sep.features.model $(MODEL_DIR)/lib_sep
	mv -f $(LIBSVM_DIR)/tools/$(FEATURE_FILE_PREFIX)-rel_open.features.model $(MODEL_DIR)/lib_rel_open
	mv -f $(LIBSVM_DIR)/tools/$(FEATURE_FILE_PREFIX)-rel_close.features.model $(MODEL_DIR)/lib_rel_close
	mv -f $(LIBSVM_DIR)/tools/$(FEATURE_FILE_PREFIX)-rela_open.features.model $(MODEL_DIR)/lib_rela_open
	mv -f $(LIBSVM_DIR)/tools/$(FEATURE_FILE_PREFIX)-rela_close.features.model $(MODEL_DIR)/lib_rela_close
	mv -f $(LIBSVM_DIR)/tools/$(FEATURE_FILE_PREFIX)-lit_open.features.model $(MODEL_DIR)/lib_lit_open
	mv -f $(LIBSVM_DIR)/tools/$(FEATURE_FILE_PREFIX)-lit_close.features.model $(MODEL_DIR)/lib_lit_close
	mv -f $(LIBSVM_DIR)/tools/$(FEATURE_FILE_PREFIX)-lit.features.model $(MODEL_DIR)/lib_lit
	mv -f $(LIBSVM_DIR)/tools/$(FEATURE_FILE_PREFIX)-rel.features.model $(MODEL_DIR)/lib_rel
	mv -f $(LIBSVM_DIR)/tools/$(FEATURE_FILE_PREFIX)-rela.features.model $(MODEL_DIR)/lib_rela

# Train the LIBSVM classifiers on the ground truth. Cleans up, extracts features and trains models.
train-classifier-lib-fix: extract-features
	rm -f $(MODEL_DIR)/lib_sep $(MODEL_DIR)/lib_rel_open $(MODEL_DIR)/lib_rel_close $(MODEL_DIR)/lib_rela_open $(MODEL_DIR)/lib_rela_close $(MODEL_DIR)/lib_lit_open $(MODEL_DIR)/lib_lit_close
	$(LIBSVM_DIR)/svm-train -t 2 -c 128     -g 0.0078125 -w1 10 -w-1 0.1 $(FEATURE_FILE_FULLPREFIX)-sep.features $(MODEL_DIR)/lib_sep
	$(LIBSVM_DIR)/svm-train -t 2 -c 32      -g 0.0078125       -w1 10 -w-1 0.1 $(FEATURE_FILE_FULLPREFIX)-rel_open.features  $(MODEL_DIR)/lib_rel_open
	$(LIBSVM_DIR)/svm-train -t 2 -c 8       -g 0.0078125       -w1 10 -w-1 0.1 $(FEATURE_FILE_FULLPREFIX)-rel_close.features $(MODEL_DIR)/lib_rel_close
	$(LIBSVM_DIR)/svm-train -t 2 -c 8       -g 0.03125         -w1 10 -w-1 0.1 $(FEATURE_FILE_FULLPREFIX)-rela_open.features $(MODEL_DIR)/lib_rela_open
	$(LIBSVM_DIR)/svm-train -t 2 -c 8       -g 0.03125         -w1 10 -w-1 0.1 $(FEATURE_FILE_FULLPREFIX)-rela_close.features $(MODEL_DIR)/lib_rela_close
	$(LIBSVM_DIR)/svm-train -t 2 -c 8       -g 0.03125         -w1 10 -w-1 0.1 $(FEATURE_FILE_FULLPREFIX)-lit_open.features $(MODEL_DIR)/lib_lit_open
	$(LIBSVM_DIR)/svm-train -t 2 -c 128     -g 0.001953125     -w1 10 -w-1 0.1 $(FEATURE_FILE_FULLPREFIX)-lit_close.features $(MODEL_DIR)/lib_lit_close
	$(LIBSVM_DIR)/svm-train -t 2 -c 8       -g 0.0078125       -b 1 -w1 1  -w-1 1   $(FEATURE_FILE_FULLPREFIX)-lit.features $(MODEL_DIR)/lib_lit
	$(LIBSVM_DIR)/svm-train -t 2 -c 32      -g 0.000488281255  -b 1 -w1 1  -w-1 1   $(FEATURE_FILE_FULLPREFIX)-rel.features $(MODEL_DIR)/lib_rel
	$(LIBSVM_DIR)/svm-train -t 2 -c 512     -g 0.0001220703125 -b 1 -w1 1  -w-1 1   $(FEATURE_FILE_FULLPREFIX)-rela.features $(MODEL_DIR)/lib_rela

train-classifier-lib-fix-large: extract-features
	rm -f $(MODEL_DIR)/lib_sep $(MODEL_DIR)/lib_rel_open $(MODEL_DIR)/lib_rel_close $(MODEL_DIR)/lib_rela_open $(MODEL_DIR)/lib_rela_close $(MODEL_DIR)/lib_lit_open $(MODEL_DIR)/lib_lit_close
	$(LIBSVM_DIR)/svm-train -t 0 -c 32      -g 0.03125         -w1 10 -w-1 0.1 $(FEATURE_FILE_FULLPREFIX)-sep.features $(MODEL_DIR)/lib_sep
	$(LIBSVM_DIR)/svm-train -t 2 -c 128     -g 0.0078125       -w1 10 -w-1 0.1 $(FEATURE_FILE_FULLPREFIX)-rel_open.features  $(MODEL_DIR)/lib_rel_open
	$(LIBSVM_DIR)/svm-train -t 2 -c 32      -g 0.0078125       -w1 10 -w-1 0.1 $(FEATURE_FILE_FULLPREFIX)-rel_close.features $(MODEL_DIR)/lib_rel_close
	$(LIBSVM_DIR)/svm-train -t 2 -c 2       -g 0.03125         -w1 10 -w-1 0.1 $(FEATURE_FILE_FULLPREFIX)-rela_open.features $(MODEL_DIR)/lib_rela_open
	$(LIBSVM_DIR)/svm-train -t 2 -c 128     -g 0.0078125       -w1 10 -w-1 0.1 $(FEATURE_FILE_FULLPREFIX)-rela_close.features $(MODEL_DIR)/lib_rela_close
	$(LIBSVM_DIR)/svm-train -t 2 -c 8       -g 0.03125         -w1 10 -w-1 0.1 $(FEATURE_FILE_FULLPREFIX)-lit_open.features $(MODEL_DIR)/lib_lit_open
	$(LIBSVM_DIR)/svm-train -t 2 -c 32      -g 0.03125         -w1 10 -w-1 0.1 $(FEATURE_FILE_FULLPREFIX)-lit_close.features $(MODEL_DIR)/lib_lit_close
	$(LIBSVM_DIR)/svm-train -t 2 -c 8       -g 0.0078125       -b 1 -w1 3  -w-1 1   $(FEATURE_FILE_FULLPREFIX)-lit.features $(MODEL_DIR)/lib_lit
	$(LIBSVM_DIR)/svm-train -t 0 -c 8       -g 0.03125         -b 1 -w1 3  -w-1 1   $(FEATURE_FILE_FULLPREFIX)-rel.features $(MODEL_DIR)/lib_rel
	$(LIBSVM_DIR)/svm-train -t 2 -c 512     -g 0.0001220703125 -b 1 -w1 3  -w-1 1   $(FEATURE_FILE_FULLPREFIX)-rela.features $(MODEL_DIR)/lib_rela
	
# Train the LIBSVM linear classifiers on the ground truth. Cleans up, extracts features and trains models.
train-classifier-lib-lin: extract-features
	rm -f $(MODEL_DIR)/lib_lin_sep $(MODEL_DIR)/lib_lin_rel_open $(MODEL_DIR)/lib_lin_rel_close \
		$(MODEL_DIR)/lib_lin_rela_open $(MODEL_DIR)/lib_lin_rela_close $(MODEL_DIR)/lib_lin_lit_open \
		$(MODEL_DIR)/lib_lin_lit_close $(MODEL_DIR)/lib_lin_lit $(MODEL_DIR)/lib_lin_rel \
		$(MODEL_DIR)/lib_lin_rela
	$(SVMLIBLEARNLIN) $(FEATURE_FILE_FULLPREFIX)-sep.features $(MODEL_DIR)/lib_lin_sep
	$(SVMLIBLEARNLIN) $(FEATURE_FILE_FULLPREFIX)-rel_open.features $(MODEL_DIR)/lib_lin_rel_open
	$(SVMLIBLEARNLIN) $(FEATURE_FILE_FULLPREFIX)-rel_close.features $(MODEL_DIR)/lib_lin_rel_close
	$(SVMLIBLEARNLIN) $(FEATURE_FILE_FULLPREFIX)-rela_open.features $(MODEL_DIR)/lib_lin_rela_open
	$(SVMLIBLEARNLIN) $(FEATURE_FILE_FULLPREFIX)-rela_close.features $(MODEL_DIR)/lib_lin_rela_close
	$(SVMLIBLEARNLIN) $(FEATURE_FILE_FULLPREFIX)-lit_open.features $(MODEL_DIR)/lib_lin_lit_open
	$(SVMLIBLEARNLIN) $(FEATURE_FILE_FULLPREFIX)-lit_close.features $(MODEL_DIR)/lib_lin_lit_close
	$(SVMLIBLEARNLIN) $(FEATURE_FILE_FULLPREFIX)-lit.features $(MODEL_DIR)/lib_lin_lit
	$(SVMLIBLEARNLIN) $(FEATURE_FILE_FULLPREFIX)-rel.features $(MODEL_DIR)/lib_lin_rel
	$(SVMLIBLEARNLIN) $(FEATURE_FILE_FULLPREFIX)-rela.features $(MODEL_DIR)/lib_lin_rela

# Label the ground truth using the SVMLight classifiers.
label-gt-light: decomposer-ml/GroundTruthClassifierMain.o
	decomposer-ml/GroundTruthClassifierMain -p $(PHRASEONLY) -f $(FEATURECONFIG_FILE) -c light $(MAP_FILE) $(MODEL_DIR)/light_sep,"SEP",$(MODEL_DIR)/light_rel_open,"REL(",$(MODEL_DIR)/light_rel_close,"REL)",$(MODEL_DIR)/light_rela_open,"RELA(",$(MODEL_DIR)/light_rela_close,"RELA)",$(MODEL_DIR)/light_lit_open,"LIT(",$(MODEL_DIR)/light_lit_close,"LIT)" $(CONTEXTGT) test

# Label the ground truth using the LIBSVM classifiers.	
label-gt-lib: decomposer-ml/GroundTruthClassifierMain.o
	decomposer-ml/GroundTruthClassifierMain -p $(PHRASEONLY) -f $(FEATURECONFIG_FILE) -c lib $(MAP_FILE) $(MODEL_DIR)/lib_sep,"SEP",$(MODEL_DIR)/lib_rel_open,"REL(",$(MODEL_DIR)/lib_rel_close,"REL)",$(MODEL_DIR)/lib_rela_open,"RELA(",$(MODEL_DIR)/lib_rela_close,"RELA)",$(MODEL_DIR)/lib_lit_open,"LIT(",$(MODEL_DIR)/lib_lit_close,"LIT)" $(CONTEXTGT) test

# Label the ground truth using the linear LIBSVM classifiers.
label-gt-lib-lin: decomposer-ml/GroundTruthClassifierMain.o
	decomposer-ml/GroundTruthClassifierMain -p $(PHRASEONLY) -f $(FEATURECONFIG_FILE) -c lib $(MAP_FILE) $(MODEL_DIR)/lib_lin_sep,"SEP",$(MODEL_DIR)/lib_lin_rel_open,"REL(",$(MODEL_DIR)/lib_lin_rel_close,"REL)",$(MODEL_DIR)/lib_lin_rela_open,"RELA(",$(MODEL_DIR)/lib_lin_rela_close,"RELA)",$(MODEL_DIR)/lib_lin_lit_open,"LIT(",$(MODEL_DIR)/lib_lin_lit_close,"LIT)" $(CONTEXTGT) test

# Run an evaluation on the test ground truth. Runs the standard classifier binary on extracted features from test ground truths and outputs accuracy.
run-eval-lib:
	# Extract features files from test ground truth but use original map files
	$(MAKE) extract-only-features MAP_FILE=$(MAP_FILE) LOCKFMAP=1 GT=$(TESTGT) CLAUSEMAP_FILE=$(CLAUSEMAP_FILE)
	# Evaluate on extracted test feature files.
	$(MAKE) eval-lib GT=$(TESTGT)

# Run the SVMLIB prediction binary using RBF models on the extracted feature files.
eval-lib: 
	cat $(FEATURE_FILE_FULLPREFIX)-sep.features | grep -E "^1" > $(FEATURE_FILE_FULLPREFIX)-sep-pos.features
	cat $(FEATURE_FILE_FULLPREFIX)-rel_open.features | grep -E "^1" > $(FEATURE_FILE_FULLPREFIX)-rel_open-pos.features
	cat $(FEATURE_FILE_FULLPREFIX)-rel_close.features | grep -E "^1" > $(FEATURE_FILE_FULLPREFIX)-rel_close-pos.features
	cat $(FEATURE_FILE_FULLPREFIX)-rela_open.features | grep -E "^1" > $(FEATURE_FILE_FULLPREFIX)-rela_open-pos.features
	cat $(FEATURE_FILE_FULLPREFIX)-rela_close.features | grep -E "^1" > $(FEATURE_FILE_FULLPREFIX)-rela_close-pos.features
	cat $(FEATURE_FILE_FULLPREFIX)-lit_open.features | grep -E "^1" > $(FEATURE_FILE_FULLPREFIX)-lit_open-pos.features
	cat $(FEATURE_FILE_FULLPREFIX)-lit_close.features | grep -E "^1" > $(FEATURE_FILE_FULLPREFIX)-lit_close-pos.features
	cat $(FEATURE_FILE_FULLPREFIX)-lit.features | grep -E "^1" > $(FEATURE_FILE_FULLPREFIX)-lit-pos.features
	cat $(FEATURE_FILE_FULLPREFIX)-rel.features | grep -E "^1" > $(FEATURE_FILE_FULLPREFIX)-rel-pos.features
	cat $(FEATURE_FILE_FULLPREFIX)-rela.features | grep -E "^1" > $(FEATURE_FILE_FULLPREFIX)-rela-pos.features
	
	@echo "SEP classifier"
	@$(SVMLIBPREDICT) $(FEATURE_FILE_FULLPREFIX)-sep.features $(MODEL_DIR)/lib_sep $(EVALDIR)/predictions_lib_sep
	@echo "REL( classifier"
	@$(SVMLIBPREDICT) $(FEATURE_FILE_FULLPREFIX)-rel_open.features $(MODEL_DIR)/lib_rel_open $(EVALDIR)/predictions_lib_rel_open
	@echo "REL) classifier"
	@$(SVMLIBPREDICT) $(FEATURE_FILE_FULLPREFIX)-rel_close.features $(MODEL_DIR)/lib_rel_close $(EVALDIR)/predictions_lib_rel_close
	@echo "RELA( classifier"
	@$(SVMLIBPREDICT) $(FEATURE_FILE_FULLPREFIX)-rela_open.features $(MODEL_DIR)/lib_rela_open $(EVALDIR)/predictions_lib_rela_open
	@echo "RELA) classifier"
	@$(SVMLIBPREDICT) $(FEATURE_FILE_FULLPREFIX)-rela_close.features $(MODEL_DIR)/lib_rela_close $(EVALDIR)/predictions_lib_rela_close
	@echo "LIT( classifier"
	@$(SVMLIBPREDICT) $(FEATURE_FILE_FULLPREFIX)-lit_open.features $(MODEL_DIR)/lib_lit_open $(EVALDIR)/predictions_lib_lit_open
	@echo "LIT) classifier"
	@$(SVMLIBPREDICT) $(FEATURE_FILE_FULLPREFIX)-lit_close.features $(MODEL_DIR)/lib_lit_close $(EVALDIR)/predictions_lib_lit_close
	@echo "LIT classifier"
	$(SVMLIBPREDICT) $(FEATURE_FILE_FULLPREFIX)-lit.features $(MODEL_DIR)/lib_lit $(EVALDIR)/predictions_lib_lit
	@echo "REL classifier"
	$(SVMLIBPREDICT) $(FEATURE_FILE_FULLPREFIX)-rel.features $(MODEL_DIR)/lib_rel $(EVALDIR)/predictions_lib_rel
	@echo "RELA classifier"
	$(SVMLIBPREDICT) $(FEATURE_FILE_FULLPREFIX)-rela.features $(MODEL_DIR)/lib_rela $(EVALDIR)/predictions_lib_rela
	
	@echo "SEP classifier POS"
	@$(SVMLIBPREDICT) $(FEATURE_FILE_FULLPREFIX)-sep-pos.features $(MODEL_DIR)/lib_sep $(EVALDIR)/predictions_lib_sep
	@echo "REL( classifier POS"
	@$(SVMLIBPREDICT) $(FEATURE_FILE_FULLPREFIX)-rel_open-pos.features $(MODEL_DIR)/lib_rel_open $(EVALDIR)/predictions_lib_rel_open
	@echo "REL) classifier POS"
	@$(SVMLIBPREDICT) $(FEATURE_FILE_FULLPREFIX)-rel_close-pos.features $(MODEL_DIR)/lib_rel_close $(EVALDIR)/predictions_lib_rel_close
	@echo "RELA( classifier POS"
	@$(SVMLIBPREDICT) $(FEATURE_FILE_FULLPREFIX)-rela_open-pos.features $(MODEL_DIR)/lib_rela_open $(EVALDIR)/predictions_lib_rela_open
	@echo "RELA) classifier POS"
	@$(SVMLIBPREDICT) $(FEATURE_FILE_FULLPREFIX)-rela_close-pos.features $(MODEL_DIR)/lib_rela_close $(EVALDIR)/predictions_lib_rela_close
	@echo "LIT( classifier POS"
	@$(SVMLIBPREDICT) $(FEATURE_FILE_FULLPREFIX)-lit_open-pos.features $(MODEL_DIR)/lib_lit_open $(EVALDIR)/predictions_lib_lit_open
	@echo "LIT) classifier POS"
	@$(SVMLIBPREDICT) $(FEATURE_FILE_FULLPREFIX)-lit_close-pos.features $(MODEL_DIR)/lib_lit_close $(EVALDIR)/predictions_lib_lit_close
	@echo "LIT classifier POS"
	$(SVMLIBPREDICT) $(FEATURE_FILE_FULLPREFIX)-lit-pos.features $(MODEL_DIR)/lib_lit $(EVALDIR)/predictions_lib_lit
	@echo "REL classifier POS"
	$(SVMLIBPREDICT) $(FEATURE_FILE_FULLPREFIX)-rel-pos.features $(MODEL_DIR)/lib_rel $(EVALDIR)/predictions_lib_rel
	@echo "RELA classifier POS"
	$(SVMLIBPREDICT) $(FEATURE_FILE_FULLPREFIX)-rela-pos.features $(MODEL_DIR)/lib_rela $(EVALDIR)/predictions_lib_rela
	

# Run the SVMLIB prediction binary using linear models on the extracted feature files.
eval-lib-lin: extract-only-features
	@echo "SEP classifier"
	@$(SVMLIBPREDICT) $(FEATURE_FILE_FULLPREFIX)-sep.features $(MODEL_DIR)/lib_lin_sep $(EVALDIR)/predictions_lib_lin_sep
	@echo "REL( classifier"
	@$(SVMLIBPREDICT) $(FEATURE_FILE_FULLPREFIX)-rel_open.features $(MODEL_DIR)/lib_lin_rel_open $(EVALDIR)/predictions_lib_lin_rel_open
	@echo "REL) classifier"
	@$(SVMLIBPREDICT) $(FEATURE_FILE_FULLPREFIX)-rel_close.features $(MODEL_DIR)/lib_lin_rel_close $(EVALDIR)/predictions_lib_lin_rel_close
	@echo "RELA( classifier"
	@$(SVMLIBPREDICT) $(FEATURE_FILE_FULLPREFIX)-rela_open.features $(MODEL_DIR)/lib_lin_rela_open $(EVALDIR)/predictions_lib_lin_rela_open
	@echo "RELA) classifier"
	@$(SVMLIBPREDICT) $(FEATURE_FILE_FULLPREFIX)-rela_close.features $(MODEL_DIR)/lib_lin_rela_close $(EVALDIR)/predictions_lib_lin_rela_close
	@echo "LIT( classifier"
	@$(SVMLIBPREDICT) $(FEATURE_FILE_FULLPREFIX)-lit_open.features $(MODEL_DIR)/lib_lin_lit_open $(EVALDIR)/predictions_lib_lin_lit_open
	@echo "LIT) classifier"
	@$(SVMLIBPREDICT) $(FEATURE_FILE_FULLPREFIX)-lit_close.features $(MODEL_DIR)/lib_lin_lit_close $(EVALDIR)/predictions_lib_lin_lit_close
	@echo "LIT classifier"
	@$(SVMLIBPREDICT) $(FEATURE_FILE_FULLPREFIX)-lit.features $(MODEL_DIR)/lib_lin_lit $(EVALDIR)/predictions_lib_lin_lit
	@echo "REL classifier"
	@$(SVMLIBPREDICT) $(FEATURE_FILE_FULLPREFIX)-rel.features $(MODEL_DIR)/lib_lin_rel $(EVALDIR)/predictions_lib_lin_rel
	@echo "RELA classifier"
	@$(SVMLIBPREDICT) $(FEATURE_FILE_FULLPREFIX)-rela.features $(MODEL_DIR)/lib_lin_rela $(EVALDIR)/predictions_lib_lin_rela
