#!/usr/bin/python
#
#    Small utility to get postags and syntactic chunks from a single sentence.
#    Just pass the sentence as a string to the command line. Make sure the
#    path to the postagger and chunker below are correct.

import sys
import getopt
import os
import subprocess
import shlex

postagger = "/home/haussmae/tagger/tree-tagger/cmd/tree-tagger-english-wiki"
chunker = "/home/haussmae/tagger/yamcha/yamcha_wrap"

def usage():
    print "Usage: "+sys.argv[0]+" <sentence> "
    
columnMode = False
try:
  opts, args = getopt.getopt(sys.argv[1:], "c", ["columns"])
except getopt.GetoptError, err:
 # print help information and exit:
  print str(err) # will print something like "option -a not recognized"
  usage()
  sys.exit(2)
output = None
verbose = False
for o, a in opts:
  if o in ("-c", "--columns"):
    columnMode = True
  else:
    assert False, "unhandled option"
    
    
    
sentence = args[0]
if (sentence == ""):
  usage()
  sys.exit(1)

p0 = subprocess.Popen([postagger], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=None)
p0.stdin.write(sentence)
output = p0.communicate()[0]

p1 = subprocess.Popen([chunker], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=None)
# This is because the chunker will only chunk the sentence if it has two newline characters at the end
p1.stdin.write(output+"\n")
output = p1.communicate()[0]

wordline = "S\t"
posline = "P\t"
npline = "N\t"
lines = output.split("\n")
for line in lines:
    if line == "" or len(line.split()) < 3:
      continue
    word, postag, npchunk = line.split()
    wordline = wordline + word + " "
    posline = posline + postag + " "
    npline = npline + npchunk + " "
print wordline.strip()
print posline.strip()
print npline.strip()
