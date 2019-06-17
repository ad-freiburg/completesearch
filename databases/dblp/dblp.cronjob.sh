#!/bin/bash
DATE_INPUTQUERIES_XML=07Aug13
DATE_INPUTQUERIES_CSV=13Sep13
HOST=`hostname`
DBLP_PORT=8181
DBLP_CSV_PORT=8182
DBLP_TEST_PORT=8189
DB=data/dblp
DB_TEST=data/dblp-test
DB_CSV=data/dblp-csv
DBLP_PATH=/home/dblp/completesearch/databases/dblp/

function printErrorSpecific()
{
  if [ "$?" -eq "$1" ] 
    then
      echo -e " ... ERROR"
      echo -e "$stderr" >&2
      exit 1
    else
      echo -e " ... OK"
  fi
}

function printError()
{
  if [ "$?" -ne "0" ] 
    then
      echo -e " ... ERROR"
      echo -e "$stderr" >&2
      exit 1
    else
      echo -e " ... OK"
  fi
}

# Change to dblp directory.
cd ${DBLP_PATH}
echo -e "Cronjob - "`date`

# Download newest dblp version.
echo -n `date +%R`" - Download newest dblp version"
stderr=`make USER=dblp DB=${DB_TEST} download 2>&1`
printError

# Build, start and test test instante of dblp completion server
echo -n `date +%R`" - Build test instance"
stderr=`make USER=dblp DB=${DB_TEST} pall 2>&1`
printError

echo -n `date +%R`" - Start test instance"
stderr=`make USER=dblp DB=${DB_TEST} PORT=${DBLP_TEST_PORT} start 2>&1`
printError

echo -n `date +%R`" - Test test instance"
cd perf
sleep 20s;
stderr=`./perf.evaluate-queries.py dblp dblp.${DATE_INPUTQUERIES_XML}.perf-input.xml-queries EGAL /home/dblp/.completesearch_${HOST}_${DBLP_TEST_PORT}.pid 2>&1`
printErrorSpecific 2
cd ..

# Stop test instance
echo -n `date +%R`" - Stop test instance"
stderr=`make USER=dblp DB=${DB_TEST} PORT=${DBLP_TEST_PORT} stop 2>&1`
printError

# Backup old data
echo -n `date +%R`" - Backup old data"
stderr=`make USER=dblp backup-dblp 2>&1`
printError

# Remove the log of the test instance, elsewhise we lose the real dblp.log when
# copying.
echo -n `date +%R`" - Remove dblp-test.log"
stderr=`rm -f ${DB_TEST}.log 2>&1`
printError

# Since everything went great copy test-binaries to original dblp-binaries,
# restart and test the original instance and remove the products of the test
# instance.
# NEW 27Aug11 (Hannah): rewrote target update-dblp-index, now simply copies all
# remaining files data/dblp-test... to data/dblp...
echo -n `date +%R`" - Update dblp instance"
stderr=`make USER=dblp DB=${DB} update-dblp-index 2>&1`
printError

echo -n `date +%R`" - Start dblp instance"
stderr=`make USER=dblp DB=${DB} PORT=${DBLP_PORT} start 2>&1`
printError

echo -n `date +%R`" - Remove dblp-test.xml"
stderr=`rm -f ${DB_TEST}.xml 2>&1`
printError

cd perf
echo -n `date +%R`" - Test instance"
sleep 20s;
stderr=`./perf.evaluate-queries.py dblp dblp.${DATE_INPUTQUERIES_XML}.perf-input.xml-queries EGAL /home/dblp/.completesearch_${HOST}_${DBLP_PORT}.pid 2>&1`
printErrorSpecific 2
cd ..

echo -n `date +%R`" - Remove dblp-test products"
stderr=`make USER=dblp DB=${DB_TEST} pclean-all 2>&1`
printError

# Download sorted xml, convert it to csv and build and start another index using it. Test it.
echo -n `date +%R`" - Download sorted xml"
stderr=`make USER=dblp DB=${DB_CSV} download-sorted-xml 2>&1`
printError

echo -n `date +%R`" - Convert xml to csv"
stderr=`make USER=dblp DB=${DB_CSV} convert-xml-to-csv 2>&1`
printError

echo -n `date +%R`" - Build csv instance"
stderr=`make USER=dblp DB=${DB_CSV} pall-csv 2>&1`
printError

echo -n `date +%R`" - Start csv instance"
stderr=`make USER=dblp DB=${DB_CSV} PORT=${DBLP_CSV_PORT} start 2>&1`
printError

cd perf
echo -n `date +%R`" - Test instance"
sleep 20s;
stderr=`./perf.evaluate-queries.py dblp dblp.${DATE_INPUTQUERIES_CSV}.perf-input.csv-queries EGAL /home/dblp/.completesearch_${HOST}_${DBLP_CSV_PORT}.pid 2>&1`
printErrorSpecific 2
cd ..

echo -e "Done - "`date +%R`"\n\n"
exit 0
