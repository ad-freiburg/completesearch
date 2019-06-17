ifndef PERF_DIR
  export PERF_DIR    := $(patsubst %/,%,$(dir $(CURDIR)/$(lastword $(MAKEFILE_LIST))))
endif

BASENAME=dblp
PORT=8181
DATE_INPUTQUERIES_XML=07Aug13
DATE_INPUTQUERIES_CSV=13Sep13
HOST=$(shell hostname)
PID=/home/dblp/.completesearch_$(HOST)_$(PORT).pid
TODAY=`date +%d%b%y`

evaluate-queries-perf-xml:
	$(PERF_DIR)/perf.evaluate-queries.py --perf-test $(BASENAME) $(PERF_DIR)/$(BASENAME).$(DATE_INPUTQUERIES_XML).perf-input.xml-queries EGAL $(PID)

evaluate-queries-perf-csv:
	$(PERF_DIR)/perf.evaluate-queries.py --perf-test $(BASENAME) $(PERF_DIR)/$(BASENAME).$(DATE_INPUTQUERIES_CSV).perf-input.csv-queries EGAL $(PID)

evaluate-queries-diff:
	$(PERF_DIR)/perf.evaluate-queries.py --diff-test $(BASENAME) $(PERF_DIR)/$(BASENAME).$(DATE_INPUTQUERIES_XML).perf-input.xml-queries $(BASENAME).$(DATE).perf-data.xml $(PID)

evaluate-queries-generate-xml:
	$(PERF_DIR)/perf.evaluate-queries.py --generate-xml-data $(BASENAME) $(PERF_DIR)/$(BASENAME).$(DATE_INPUTQUERIES_XML).perf-input.xml-queries $(BASENAME).$(DATE).perf-data.xml $(PID)

generate-queries:
	$(PERF_DIR)/perf.generate-queries.sh $(PERF_DIR)/$(BASENAME).$(TODAY).perf-input.query-seeds

grep-queries:
	cat -A /var/log/dblp/dblp.log | grep completions | awk '{print $$2" "$$3}' | grep -v "empty query" | sed -e 's/\^\[\[1m//g' \
	-e "s/[[:digit:]]\+\.[[:digit:]]\+//g" -e "s/M-CM-.//g" | tail -n 5000 > $(PERF_DIR)/$(BASENAME).$(TODAY).perf-input.xml-queries
	cat $(PERF_DIR)/$(BASENAME).$(TODAY).perf-input.queries | sed -e "s/_/ :filter:author:/g" -e "s/^ce:author:/:filter:/g" \
	-e "s/ct:/:facet:/g" -e "s/\(:filter:\w\+:\w\+\):\*/\1/g" -e "s/\([^\*]\) /\1* /g" > $(PERF_DIR)/$(BASENAME).$(TODAY).perf-input.csv-queries



