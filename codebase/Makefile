# Makefile for COMPILING THE SOURCE CODE
#
# This builds the following binaries. See applications/Makefile for how these
# binaries are used.
#
# server/startCompletionServer	start server
# server/buildIndex		build index for search
# server/buildDocsDB		build index for document excerpts
# parser/CsvParserMain		parse CSV file and produce input files for
# 				buildIndex and buildDocsDB
#
# The following software needs to be installed besides Completesearch. See the
# Dockerfile for more details.
#  * Google Test 1.6.0 (http://code.google.com/p/googletest/)
#  * Google hash map (http://code.google.com/p/google-sparsehash/)
#  * Expat XML parser (apt-get install libexpat1-dev)
#  * Zlib compression library (apt-get install zlib1g-dev)
#  * Stxxl external sorting library (http://stxxl.sourceforge.net/)

ifndef CS_CODE_DIR
  export CS_CODE_DIR    := $(patsubst %/,%,$(dir $(CURDIR)/$(lastword $(MAKEFILE_LIST))))
endif

CS_BIN_DIR       = $(CS_CODE_DIR)/server
STXXL_SORT_DIR   = $(CS_CODE_DIR)/binarysort
STXXL_CONFIG     = /opt/stxxl/stxxl.mk
LIBS_INCLUDED    = -L$(CS_CODE_DIR)/gtest
TEST_REPORTS_DIR = $(CS_CODE_DIR)/test-reports
MAPS_DIR         = $(CS_CODE_DIR)/utility

PARSER         = $(CS_CODE_DIR)/parser/CsvParserMain
PARSER_OPTIONS =
XML_OPTS       =
SORT           = sort
LOCALE_POSIX   = export LC_ALL=POSIX
SHELL         := /bin/bash
PERL           = perl
PLATFORM       = $(shell uname -s)
CPPLINT        = $(CS_CODE_DIR)/utility/cpplint/cpplint.py


# Compiler and flags.
CXX_OPTIONS  = --std=c++11 -Wall# -Wno-uninitialized -Wno-deprecated -Wno-unused-function -fexceptions
# CXX_OPTIONS  = -Wall -Wno-uninitialized -Wno-deprecated -Wno-unused-function -fexceptions --std=gnu++0x
CXX_DEFINES  = -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_REENTRANT -DSTL_VECTOR -DDEBUG_PTHREAD_CREATE_TIME
CXX_DEBUG    = -O3# -g #-DNDEBUG
CXX_INCLUDES = -I$(CS_CODE_DIR) -I$(CS_CODE_DIR)/gtest/include
# NOTE(Hannah, 26-01-2017): gtest uses tuple from different dir than sparsehash.
CXX_FIXES = -DGTEST_HAS_TR1_TUPLE=0 -DGTEST_USE_OWN_TR1_TUPLE=0
CXX = g++ $(CXX_OPTIONS) $(CXX_DEBUG) $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FIXES)

# All test and perf files (will be expanded only when used, so can be used in
# sub-directory).
TEST_BINARIES = $(basename $(wildcard *Test.cpp))
PERF_BINARIES = $(basename $(wildcard *Perf.cpp))

# Build code in every sub-directory.
build-all:
	echo $(CS_CODE_DIR)
	rm -f $(CS_CODE_DIR)/server/StartCompletionServer.o
	$(MAKE) -C $(CS_CODE_DIR)/gtest build
	$(MAKE) -C $(CS_CODE_DIR)/synonymsearch build
	$(MAKE) -C $(CS_CODE_DIR)/utility build
	$(MAKE) -C $(CS_CODE_DIR)/fuzzysearch build
	$(MAKE) -C $(CS_CODE_DIR)/binarysort build
	$(MAKE) -C $(CS_CODE_DIR)/parser build
	$(MAKE) -C $(CS_CODE_DIR)/server build

# Test code in every sub-directory.
test-all:
	$(MAKE) -C $(CS_CODE_DIR)/binarysort test
	$(MAKE) -C $(CS_CODE_DIR)/synonymsearch test
	$(MAKE) -C $(CS_CODE_DIR)/utility test
	$(MAKE) -C $(CS_CODE_DIR)/parser test
	$(MAKE) -C $(CS_CODE_DIR)/server test

# Remove all build products (object files, binaries, etc.) in all
# sub-directories.
clean-all:
	$(MAKE) -C $(CS_CODE_DIR)/gtest clean
	$(MAKE) -C $(CS_CODE_DIR)/parser clean
	$(MAKE) -C $(CS_CODE_DIR)/binarysort clean
	$(MAKE) -C $(CS_CODE_DIR)/fuzzysearch clean
	$(MAKE) -C $(CS_CODE_DIR)/synonymsearch clean
	$(MAKE) -C $(CS_CODE_DIR)/utility clean
	$(MAKE) -C $(CS_CODE_DIR)/server clean
	rm -f $(TEST_REPORTS_DIR)/*.xml

# Generic build target that should work in every sub-directory.
build:
	$(MAKE) $(BINARIES) $(TEST_BINARIES) $(PERF_BINARIES) $(LIBS)

# Generic checkstyle target that should work in every sub-directory.
checkstyle:
	python $(CPPLINT) *.h *.cpp

# Generic test target that should work in every sub-directory.
test:
	mkdir -p $(TEST_REPORTS_DIR)
	for T in *Test *Test.exe; \
	  do [[ $${T:0:1} == "*" ]] && continue; \
	  export LD_LIBRARY_PATH=$(GTEST_LIBS); ./$$T --gtest_output=xml; \
	  TESTNAME="`basename $${T} .exe`"; \
	  mv test_detail.xml $${TESTNAME}.xml; \
	  $(PERL) $(CS_CODE_DIR)/utility/gtest-hudson.pl \
	    $${TESTNAME}.xml $(TEST_REPORTS_DIR); \
		rm -f $${TESTNAME}.xml; done

# Generic perf target that should work in every sub-directory.
perf:
	for P in *Perf; \
	  do ./$$P; done

# Generic clean target that should work in every sub-directory.
clean:
	rm -f *.o
	rm -f *Main.exe *Test.exe *Perf.exe
	rm -f $(BINARIES) $(TEST_BINARIES) $(PERF_BINARIES) $(LIBS)

# Compute test coverage.
COVERAGE_NAME="coverage.xml"
check-all:
	$(MAKE) "CXX_DEBUG=-O0 -g -fprofile-arcs -ftest-coverage -fPIC -lgcov" clean-all build-all test-all
	$(MAKE) -C $(CS_CODE_DIR)/example/xml/ test-pall-xml
	$(MAKE) -C $(CS_CODE_DIR)/example/csv/ test-pall-csv
	$(CS_CODE_DIR)/utility/gcovr -x -d -r . -e $(CS_CODE_DIR)/gtest/ -e $(CS_CODE_DIR)/test-reports/ -e '.*Test' > $(COVERAGE_NAME)
	# Clean cobertura output "dir...." should be erased, same for "dir/../",
	# "./" and ".." at the end of the line
	sed -i 's/\w*\.\.\.\.//g' $(COVERAGE_NAME)
	sed -i 's/\w*\/\.\.\/*//g' $(COVERAGE_NAME)
	sed -i 's/\.\///g' $(COVERAGE_NAME)
	sed -i 's/\.\.\">$$/\">/g' $(COVERAGE_NAME)
	python $(CPPLINT) */*.cpp */*.h &> cpplint.xml || echo 0
