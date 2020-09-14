#!/bin/bash
IFS=$'\012'
DATUM=`date +%d%b%y`
FILENAME="dblp."${DATUM}".perf-input.queries"
COLLECTION="dblp"
function usage()
{
  echo -e "Usage info:"
  echo -e "\t./perf.generate-queries.sh <list of queries>"
  echo -e "Options:"
  echo -e "\t--append-suffixes=<collection_name>"
  echo -e "\timplemented right now: dblp"
  exit 0
}

function append-suffixes()
{
  if [ "$2" = "dblp" ]
  then
    echo $1*\&c=0\&h=20
    echo $1*\&c=4\&h=0
    echo $1* ct:venue:*\&c=4\&h=0
    echo $1* ct:author:*\&c=4\&h=0
    echo $1* ct:year:*\&c=4\&h=0
  else
    echo ERROR: Collection $2 is not yet implemented. >&2
    exit 1 
  fi
}

function transform()
{
  query=$1
  for (( i=0; i<${#query}; i++ ))
  do
    if [ "${query:$i:1}" = ' ' ]
    then
      query=${query:0:$i}"*"${query:$i}
      i=$(($i+2))
    fi
  done
  if [ "${#query}" -le "3" ]
  then
    random=4
  else
    random=$((($RANDOM % (${#query} - 3)) + 4))
  fi
  for (( i=4; i<=$random; i++ ))
  do
    if [ "${query:$i:1}" = ' ' ] 
    then
       i=$(($i+4))
    fi
    if [ "${query:$i-1:1}" = '*' ] 
    then
      i=$(($i+2))
    fi
    append-suffixes "${query:0:$i}" $COLLECTION
  done
}

echo "Queries are going to be written into file \"${FILENAME}\"."
top=`getopt -n "$0" -l "append-suffixes:" -o "a:" -- "$@"`
if [ $? -ne 0 ] 
then
  usage
fi
eval set -- $top
if [ $# -eq 0 ]
then
  usage
fi

listfile=''
while [ true ]
do
  case "$1" in
   --append-suffixes|-a) echo "Collection: "$2;COLLECTION=$2;shift 2;;
   --)                shift;;
   -*)                usage;;
    *)                listfile=$1; break;; #query lisz
  esac
done

#if [ $# -lt 1 ]
#then
#  usage
#fi

true > $FILENAME
query=''
for q in $(cat $listfile)
do
  echo $q
  transform $q >> $FILENAME
done

