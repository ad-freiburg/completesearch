#!/home/haussmae/pypy/pypy-1.6/bin/pypy
#/usr/bin/python

import codecs
import sys
from collections import deque

fileName = sys.argv[1]
print "Opening %s"%(fileName)
fileObj = codecs.open( fileName, "r", "utf-8")
lastlines = deque(maxlen=10)

#size = file.tell()


try:
  for line in fileObj:
    lastlines.append(line)
    words = line.split('\t')
    word  = words[0]
    rest = words [1:]
    print word.encode('utf-8')
    print rest
    next
except:
  print "Error. Previous lines:"
  for li in lastlines:
    print li
  raise


