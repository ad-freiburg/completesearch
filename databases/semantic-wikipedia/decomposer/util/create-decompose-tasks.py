#!/usr/bin/python

import codecs
import sys
import glob
from collections import deque

db = sys.argv[1]
taskFile = open(db+".task", "w")
splits = glob.glob(db+".words-unsorted.prechunk.tagged.ascii.split[0-9][0-9][0-9]")
splits.sort()

for file in splits:
  newFile = file
  taskFile.write("WORDS_UNSORTED_FILE="+file+"\n")
