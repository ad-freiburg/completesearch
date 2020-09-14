#!/home/haussmae/pypy/pypy/bin/pypy
import itertools
import mmap
import re
import os

SENTENCE_SPLIT = "<__EOSENTENCE>"

def consume(iterator, n):
  "Advance the iterator n-steps ahead. If n is none, consume entirely."
  # Use functions that consume iterators at C speed.
  if n is None:
      # feed the entire iterator into a zero-length deque
      collections.deque(iterator, maxlen=0)
  else:
      # advance to the empty slice starting at position n
      next(itertools.islice(iterator, n, n), None)

  
class indexwalk_iter:
  def __init__(self, indexIter):
    self.indexIter = indexIter
    self.end = False
    self.lastLine = ""
    self.sentenceId = -1
  def __iter__(self):
    return self                 # simplest iterator creation
  def next(self):
    npChunkIterCopy = self.indexIter # = itertools.tee(self.indexIter)
    sentence = []
    if (self.end):
        raise StopIteration
    try:
      # for all but the first sentence we read one line ahead.
      if self.sentenceId != -1:
        word, docId, score, position = self.lastLine.rstrip().split('\t')
        sentence.append((word, docId, score, position))
      while True:
          npChunkLine = self.indexIter.next()
          try:
            word, docId, score, position = npChunkLine.rstrip().split('\t')
          except:
            print "Error splitting line in original index"
            print "Current line:"
            print npChunkLine
            print "Last line:"
            print self.lastLine
            exit(1)
          self.lastLine = npChunkLine
          if self.sentenceId == -1:
              self.sentenceId = docId
          elif docId != self.sentenceId:
              self.sentenceId = docId
              break
          sentence.append((word, docId, score, position))
    except StopIteration:
        self.end = True
        return sentence
    return sentence          
    

class tagwalk_iter:
  def __init__(self, indexIter):
    self.indexIter = indexIter
    self.end = False
  def __iter__(self):
    return self
  def next(self):
    if (self.end):
      raise StopIteration
    sentence = []
    try:
        while True:
            npChunkLine = self.indexIter.next()
            # npChunkLine = npChunkLine.strip()
            if npChunkLine.startswith(SENTENCE_SPLIT):
                break
            try:
              token = npChunkLine.rstrip().split('\t')
            except:
              print "Error splitting line in tagged index"
              print npChunkLine
              exit(1)
            sentence.append(token)     
        return sentence
    except StopIteration:
        self.end = True
        return sentence

def tail(filename, n):
    """Returns last n lines from the filename. No exception handling"""
    size = os.path.getsize(filename)
    with open(filename, "rb") as f:
        # for Windows the mmap parameters are different
        fm = mmap.mmap(f.fileno(), 0, mmap.MAP_SHARED, mmap.PROT_READ)
        try:
            for i in xrange(size - 1, -1, -1):
                if fm[i] == '\n':
                    n -= 1
                    if n == -1:
                        break
            return fm[i + 1 if i else 0:].splitlines()
        finally:
            fm.close()

