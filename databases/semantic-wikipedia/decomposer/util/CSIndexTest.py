#!/home/haussmae/pypy/pypy/bin/pypy
import sys
import time
import itertools
import copy
import re
import codecs
import os
import logging
from CSIndex import *

def testIndexReads():
    print ("Starting to incorporate POS-tags into original index.")
    startCombinePosTag = time.time()
    # Open files.
    indexFile = open(indexFilename,"r")
    npChunkFile = open(tmpChunkFilename,"r")
    # Standard iterators.
    npChunkIter = iter(npChunkFile)
    indexIter = iter(indexFile)
    # Custom iterators.
    npTagSentenceIter = tagwalk_iter(npChunkIter)
    indexSentenceIter = indexwalk_iter(indexIter)
    errors = 0
    nSentences = 0
    for indexSentence in indexSentenceIter:
        nSentences += 1
        print "Next sentence is this:"
        for line in indexSentence:
            sWord, docId, score, position = line          
            print("%s %s %s %s"%(sWord, docId, score, position))
        if nSentences > 10:
          print "Ending in test"
          break
    # Close files.
    indexFile.close()
    npChunkFile.close()
    endCombinePosTag = time.time()
    print("An error occured in %i sentences (%i total)"%(errors, nSentences))
    print("Done(%.1f s)." %(endCombinePosTag - startCombinePosTag))

indexFilename = sys.argv[1]
tmpChunkFilename = sys.argv[2]

print "Using index file: " + indexFilename

startAll = time.time()
testIndexReads()
endAll = time.time()
print("Done. Overall %.1f ms." % (endAll - startAll))
