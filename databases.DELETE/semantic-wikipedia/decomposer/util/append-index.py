#!/home/haussmae/pypy/pypy/bin/pypy

from CSIndex import *
import os
import sys

def printUsage():
  print "Usage: "+sys.argv[0]+""" wordsIndexFilename wordsAppendixFilename
  docsIndexFilename docsAppendixFilename"""
  print "Example: "+sys.argv[0]+""" 
          semantic-wikipedia.words-unsorted.prechunk.ascii 
          semantic-wikipedia.words-unsorted.ascii.appendix 
          semantic-wikipedia.docs-unsorted.prechunk.ascii
          semantic-wikipedia.docs-unsorted.ascii.appendix"""

def getHighestDocIdFromWordsFile(filename):
  if (not os.path.exists(filename)):
    return 0
  if (os.stat(filename).st_size == 0):
    return 0;
  testFile = open(filename)
  test = tail(filename, 1)
  word, docId, score, position = test[0].split('\t')
  return int(docId)

def appendToDocsFile(docsFilename, docsAppendixFilename, docId):
  outFile = open(docsFilename, "a")
  indexFile = open(docsAppendixFilename,"r")
  for line in indexFile:
    origDocId, url, title, sentence = line.split('\t')
    outFile.write(str(docId)+"\t"+url+"\t"+title+"\t"+sentence)
    docId = docId + 1
  outFile.close()
  return docId - 1

def appendToWordsFile(wordsFilename, wordsAppendixFilename, docId):
  outFile = open(wordsFilename, "a")
  indexFile = open(wordsAppendixFilename,"r")
  indexIter = iter(indexFile)
  indexSentenceIter = indexwalk_iter(indexIter)
  for sentence in indexSentenceIter:
    for line in sentence:
      word, origDocId, score, position = line          
      outFile.write(word+"\t"+str(docId)+"\t"+score+"\t"+position+"\n")
    docId = docId + 1
  outFile.close()
  return docId - 1

if (len(sys.argv) < 5):
  printUsage()
  exit(1)

wordsIndexFilename = sys.argv[1]
wordsAppendixFilename = sys.argv[2]
docsIndexFilename = sys.argv[3]
docsAppendixFilename = sys.argv[4]

print "Using words index file: " + wordsIndexFilename
print "Using words appendix file: " + wordsAppendixFilename
print "Using docs index file: " + docsIndexFilename
print "Using docs appendix file: " + docsAppendixFilename
docIdStart = getHighestDocIdFromWordsFile(wordsIndexFilename)+1
print "Next Document ID: " + str(docIdStart)

lastDocId = appendToWordsFile(wordsIndexFilename, wordsAppendixFilename, docIdStart)
print "Last Doc ID in words file: " + str(lastDocId)
lastDocId = appendToDocsFile(docsIndexFilename, docsAppendixFilename, docIdStart)
print "Last Doc ID in docs file: " + str(lastDocId)

