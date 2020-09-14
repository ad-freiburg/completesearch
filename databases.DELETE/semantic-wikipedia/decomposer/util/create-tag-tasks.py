#!/usr/bin/python

import codecs
import sys
import glob
from collections import deque

db = sys.argv[1]
taskFile = open(db+".task", "w")
splits = glob.glob(db+".words-unsorted.prechunk.ascii.split*")
splits.sort()

for file in splits:
  newFile = file.replace("ascii","tagged.ascii")
  taskFile.write(newFile+"\n")
