include ../../Makefile

do-build:
	$(CXX) LogTrialMacros.cpp -o LogTrialMacros
	$(CXX) LogTrialMacrosCompilerSet.cpp -DLOGLEVEL=1 -o LogTrialMacrosCompilerSet
	$(CXX) LogTrialTemplates.cpp -o LogTrialTemplates
	
run: do-build
	./LogTrialMacros
	./LogTrialMacrosCompilerSet