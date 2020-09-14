#!/usr/bin/python

import codecs
import sys
import glob
from collections import deque

db = sys.argv[1]
taskFile = open(db+".task", "w")
splits = glob.glob(db+".words-unsorted.prechunk.tagged.ascii.split*.decomposed")
splits.sort()

for file in splits:
  docsFile = file.replace("decomposed","docs-unsorted")
  taskFile.write(db+".words-unsorted.ascii "+file + " " + db+".docs-unsorted " +docsFile+"\n")
