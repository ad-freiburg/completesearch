#!/usr/bin/python
import sys
import time
import itertools
import copy
import re
import codecs
import os

def printUsage():
  print "Usage: "+sys.argv[0]+" indexFilename outputFilename regex"
  print "Example: "+sys.argv[0]+" semantic-wikipedia.words-unsorted.ascii tagged \"^ct:\""

def splitAtRe():
    print ("Starting to read index file")
    startCombinePosTag = time.time()
    indexFile = open(indexFilename,"r", 16 << 10)
    outFile = open(outFilename,"w")
    size = os.path.getsize(indexFilename)
    indexFile.seek(size-size/10)
    # indexFile.seek(size-size/10)
    splitFound = False
    for line in indexFile:
      if SPLITRE.match(line):
        print ("Split found, starting to write outfile")
        splitFound = True
        outFile.write(line)
        break
    for line in indexFile:
      outFile.write(line)
    outFile.close() 
    indexFile.close()
    endCombinePosTag = time.time()
    print("Done(%.1f s)." %(endCombinePosTag - startCombinePosTag))

if (len(sys.argv) < 4):
  printUsage()
  exit(1)

indexFilename = sys.argv[1]
outFilename = sys.argv[2]
splitRegex = sys.argv[3]
SPLITRE = re.compile(splitRegex)

print "Using split regex: " + splitRegex
print "Using output file: " + outFilename
print "Using index file: " + indexFilename

startAll = time.time()
splitAtRe()
endAll = time.time()
print("Done. Overall %.1f ms." % (endAll - startAll))
