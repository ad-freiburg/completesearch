include ../Makefile

do-build: StringUtilsTest ComparatorsTest FileTest

do-test: StringUtilsTest ComparatorsTest FileTest
	./StringUtilsTest
	./ComparatorsTest
	./FileTest
	
do-clean: 
	rm -f StringUtilsTest
	rm -f ComparatorsTest
	rm -f FileTest

%.o: %.cpp $(HEADERS)
	$(CXX) -c $<	 
		
%Test: %Test.cpp $(HEADERS) $(LIBGTEST_A)
	$(CXX_TEST) -o $@ $< $(TEST_INCLUDE)
	
