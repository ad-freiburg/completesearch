#!/home/haussmae/pypy/pypy/bin/pypy

#/usr/bin/python
import sys
import time
import itertools
import copy
import re
import codecs
import os
import logging
from CSIndex import *


EMPTY_TAG = '*'
WINDOW_LENGTH = 20
IGNORE_REGEX = re.compile("(:.+|ct:|__eofentity).*")
REMOVE_REGEX = re.compile("<.*?>") 
LOGLEVEL = logging.WARN
NUMTAGS = 4


def normalizeUnicodeWord(word):
  """Replace certain UTF-8 characters with ASCII ones, because the POS
  tagger usually does so."""
  # Non-breaking whitespace.
  normalizedWord  = word.replace(u'\u00a0','')
  # EM_Dash --
  normalizedWord = normalizedWord.replace(unichr(0x2014),'--')
  # EM_Dash --
  normalizedWord = normalizedWord.replace(unichr(0x2013),'--')
  # RIGHT DOUBLE QUOTATION MARK
  normalizedWord = normalizedWord.replace(unichr(0x201d),'\'\'')
  # LEFT DOUBLE QUOTATION MARK
  normalizedWord = normalizedWord.replace(unichr(0x201c),'\'\'')
  # LEFT SINGLE QUOTATION MARK
  normalizedWord = normalizedWord.replace(unichr(0x2018),'\'')
  # RIGHT SINGLE QUOTATION MARK
  normalizedWord = normalizedWord.replace(u'\u2019','\'')
  # HORIZONTAL ELLIPSIS
  normalizedWord = normalizedWord.replace(u'\u2026','...')
  if (normalizedWord != word):
    log.debug("Changed '%s' to '%s'"%(word.encode('unicode_escape'),
      normalizedWord.encode('unicode_escape')))
  return normalizedWord

def normalizeTaggerWord(word):
  """Undo tagger normalization, e.g. transform '-LRB-' back to '('."""
  if (word == "-RRB-"):
    word = ")"
  elif (word == "-LRB-"):
    word = "("
  elif (word == "``"):
    word = "''"
  elif (word == "`"):
    word = "'"
  return word

def removeParserErrors(word):
  """Sometimes the parser leaves some errors like tags which we remove."""
  nWord = re.sub(REMOVE_REGEX,"",word)
  if (nWord != word):
    log.debug("Changed '%s' to '%s'"%(word.encode('unicode_escape'),
      nWord.encode('unicode_escape')))
  return nWord

def mydistance(s,t):
    """ A trivial implementation to compute the edit distance between s and t
    """
    # d is a table with u rows and v columns
    u = len(s) + 1
    v = len(t) + 1
    d = [[0 for x in range (0,v)] for y in range(0,u)]
    for i in range(0,u):
        d[i][0] = i # deletion
    for j in range(0,v):
        d[0][j] = j # insertion
    for j in range(1,v):
        for i in range(1,u):
            if (s[i-1] == t[j-1]):
                d[i][j] = d[i-1][j-1]
            else:
                d[i][j] = min(d[i-1][j] + 1, d[i][j-1] + 1, d[i-1][j-1] + 1)
    return d[len(s)][len(t)]

def initLog():
  log.setLevel(LOGLEVEL)
  logging.basicConfig()

# npChunkerBinary takes inputFile as parameter and outputs to stdout
# posTaggerBinary takes inputFile as parameter and outputs to stdout
def printUsage():
  print "Usage: "+sys.argv[0]+" indexFilename posTaggerBinary npChunkerBinary outputFilename"
  print "Example: "+sys.argv[0]+""" semantic-wikipedia.words-unsorted.ascii 
          ~/tree-tagger/cmd/tree-tagger-english-wiki ~/yamcha/yamcha_wrap
          ./semantic-wikipedia-test.words-unsorted-postagged"""

def createTempIndexFile():
  """ Create a temporary index file that
  only contains proper words"""
  print "Creating temporary index file (w/o entities etc.)."
  indexFile = open(indexFilename,"r")
  tempFile = open(tmpFilename,"w")
  currentDocId = -1
  words = 0
  sentences = 0
  startTempIndex = time.time()
  for line in indexFile:
    word, docId, score, position = line.split('\t')
    # Treat as UTF-8.
    word = word.decode('utf-8')
    # Skip empty words.
    if (word == ""):
      continue
    # New sentence. Write sentence separator to finish previous sentence.
    if (docId != currentDocId and currentDocId != -1):
      tempFile.write(SENTENCE_SPLIT+"\n")
      sentences += 1
    # If IGNORE_REGEX matches (.e.g for categories and special words)
    # skip this word.
    if(not IGNORE_REGEX.match(word)):
      # fix for <> tags that are still in the index (but will be removed)
      word = removeParserErrors(word)
      word = normalizeUnicodeWord(word)
      # Encode back to UTF-8 but write as byte-string.
      tempFile.write(str((word+"\n").encode('utf-8')))
      words +=1
    currentDocId = docId
  # Finish last sentence.
  tempFile.write("\n")
  tempFile.write(SENTENCE_SPLIT+"\n")
  tempFile.close()
  sentences += 1
  endTempIndex = time.time()
  print("Done(%.1f s). %s Words in %s sentences" % ((endTempIndex -
    startTempIndex),words, sentences))

def posTagTempIndexFile(inputFileName, outputFileName):
  """POS-tag the temporary index file. 
  POS-tagging MUST NOT tokenize to create new sentence"""
  # ignore the sentence annotations in the temporary output file
  print "Starting to POS-tag temporary index file."
  startPosTag = time.time()
  os.system(posTagger+" "+inputFileName+" > "
      +outputFileName)
  endPosTag = time.time()
  print("Done(%.1f s)." % (endPosTag - startPosTag))

def npChunkTempIndexFile(inputFileName, outputFileName):
  """NP-Chunk the previously POS-tagged temporary index file"""
  print "Starting to NP-Chunk the pos-tagged index file"
  startNpChunk = time.time()
  print "cd /home/haussmae/systems/senna/senna-v3.0/;"+ npChunker + "<" + inputFileName + " > " + outputFileName
  os.system("cd /home/haussmae/systems/senna/senna-v3.0/;"+ npChunker + "<" +
      inputFileName + " > " + outputFileName)
  endNpChunk = time.time()
  print("Done(%.1f s)." % (endNpChunk - startNpChunk))

def combinePosTagAndOriginalBySentence(indexFileName, taggedFileName,
    outputFileName):
    print ("Starting to incorporate POS-tags into original index.")
    startCombinePosTag = time.time()
    # Open files.
    indexFile = open(indexFilename,"r")
    npChunkFile = open(taggedFileName,"r")
    outFile = codecs.open(outputFileName,"w","utf-8")
    # Standard iterators.
    npChunkIter = iter(npChunkFile)
    indexIter = iter(indexFile)
    # Custom iterators.
    npTagSentenceIter = tagwalk_iter(npChunkIter)
    indexSentenceIter = indexwalk_iter(indexIter)
    # Error counter.
    errors = 0
    # Count number of sentences.
    nSentences = 0
    for indexSentence in indexSentenceIter:
        nSentences += 1
        # get the next sentence from the tagged file
        tagSentence = npTagSentenceIter.next()
        currentTagLine = 0
        # If we observe an error this causes a skip to the end of the sentence.
        skipSentence = False
        # For each line in the index file.
        for line in indexSentence:
            # Split the line.
            sWord, docId, score, position = line          
            # We read as byte-string. Now decode from UTF-8.
            word = sWord.decode('utf-8')
            # Fix errors.
            word = re.sub(REMOVE_REGEX, "", word)
            # Skip empty words.
            if (word == ""):
              continue
            # This is a special word that was ignored before.
            if(IGNORE_REGEX.match(word)):
              tagString = ''.join(["\t"+EMPTY_TAG for i in xrange(NUMTAGS)]);
              outFile.write(word+"\t"+docId+"\t"+score+"\t"+position+
                  tagString+"\n")
              continue
            # If we observed an error write the word with empty tags and
            # continue.
            if (skipSentence):
              tagString = ''.join(["\t"+EMPTY_TAG for i in xrange(NUMTAGS)]);
              outFile.write(word+"\t"+docId+"\t"+score+"\t"+position+
                  tagString+"\n")
              continue
            # Read next line in tagged sentence.
            if currentTagLine < len(tagSentence):
              sPosWord = tagSentence[currentTagLine][0]
              tags = tagSentence[currentTagLine][1:]
              posWord = sPosWord.decode('utf-8')
              posWord = normalizeTaggerWord(posWord)
            else:
              # This might happen if the tagger further split the sentence
              # and as a consequence one index sentence corresponds to two 
              # tagged sentences.
              log.warn("Tried to read after end of tagged sentence.")
              log.warn("It seems the index sentence is longer.")
              log.warn("Tag Sentence: " + str(tagSentence))
              log.warn("Index Sentence: " + str(indexSentence))
              errors += 1
              skipSentence = True
              continue
            # Normalize word.
            word = normalizeUnicodeWord(word)
            # Try to match tagged word with original index word.
            if (posWord == word):
                # We matched. Write the word - advance to next one in tagged
                # index.
                tagString = ''.join(["\t"+s for s in tags]);
                outFile.write(word+"\t"+docId+"\t"+score+"\t"+position+tagString+"\n")
                currentTagLine += 1    
            # There was no exact match.
            else:
                # Remember the tagged word we start with.
                editTagLine = currentTagLine;
                log.debug("Not matched %s against %s" %(posWord, word))
                combinePosWord = u""
                # In each step append the next tagged word to form a combined word 
                # and see if the result matches the original index word.
                # Tagged words range from current to last word of tagged sentence.
                for i in range(0, len(tagSentence)-(currentTagLine+1)):
                    sPosWord = tagSentence[currentTagLine + i][0]
                    tags = tagSentence[currentTagLine + i][1:]
                    posWord = normalizeTaggerWord(sPosWord.decode('utf-8'))
                    combinePosWord += posWord
                    # The final case. The combined word matches.
                    if (word == combinePosWord):
                        log.debug("Match combined %s against %s"%(combinePosWord, word))
                        currentTagLine += (i+1)
                        log.debug("Fixed and continuing.")
                        tagString = ''.join(["\t"+s for s in tags]);
                        outFile.write(posWord+"\t"+docId+"\t"+score+"\t"+position+tagString+"\n")
                        break
                    # The first and intermediate case. The pos tagged combined word is at the start.
                    elif (word.startswith(combinePosWord)):
                        log.debug("Match combined %s at start of %s"%(combinePosWord, word))
                        tagString = ''.join(["\t"+s for s in tags]);
                        outFile.write(posWord+"\t"+docId+"\t"+score+"\t"+position+tagString+"\n")
                    # The error case if the words cannot be combined
                    # (trivially).
                    else:
                        sPosWord = tagSentence[editTagLine][0]
                        tags = tagSentence[editTagLine][1:]
                        posWord = sPosWord.decode('utf-8')
                        if (mydistance(posWord, word) < 3):
                          tagString = ''.join(["\t"+s for s in tags]);
                          outFile.write(word+"\t"+docId+"\t"+score+"\t"+position+tagString+"\n")
                          log.debug("Fixed and continuing.")
                          currentTagLine = editTagLine + 1
                          break
                        else:
                          log.warn("Could not match tagged word '%s' against index word '%s'"%(combinePosWord.encode('unicode_escape'), word.encode('unicode_escape')))
                          log.debug("Tag Sentence: " + str(tagSentence))
                          log.debug("Index Sentence: " + str(indexSentence))
                          log.warn("Continuing with next sentence.")
                          errors += 1
                          # This causes the remaining special words of the
                          # sentence to be written.
                          skipSentence = True
                          break
    # Close files.
    outFile.close() 
    indexFile.close()
    npChunkFile.close()
    endCombinePosTag = time.time()
    print("An error occured in %i sentences (%i total)"%(errors, nSentences))
    print("Done(%.1f s)." %(endCombinePosTag - startCombinePosTag))

if (len(sys.argv) < 5):
  printUsage()
  exit(1)

indexFilename = sys.argv[1]
posTagger = sys.argv[2]
npChunker = sys.argv[3]
outFilename = sys.argv[4]
tmpFilename = outFilename+".tmp"
tmpPOSFilename = outFilename+".pos.tmp"
tmpChunkFilename = outFilename+".pos.chunk.tmp"

log = logging.getLogger("tagchunk-index")
initLog()

print "Using index file: " + indexFilename
print "Using POS-Tagger: " + posTagger
print "Using NP-Chunker: " + npChunker
print "Using output file: " + outFilename

startAll = time.time()
createTempIndexFile()

if (posTagger == "NONE"):
  npChunkTempIndexFile(tmpFilename, tmpChunkFilename)
else:
  posTagTempIndexFile(tmpFilename, tmpPOSFilename)
  npChunkTempIndexFile(tmpPOSFilename, tmpChunkFilename)

combinePosTagAndOriginalBySentence(indexFilename, tmpChunkFilename,
    outFilename)
#test()
endAll = time.time()
print("Done. Overall %.1f ms." % (endAll - startAll))

print "Deleting temporary files"
#os.remove(tmpFilename)
#os.remove(tmpPOSFilename)
#os.remove(tmpChunkFilename)
