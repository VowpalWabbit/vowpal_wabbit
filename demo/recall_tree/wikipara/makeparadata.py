#! /usr/bin/env python

# about 1.8 million documents (3x training examples)
# 
# 11502.9 1796001 3130318 0.364573

import os
import random
import struct
import string
import sys
import time

import DocGenerator

random.seed (90210)

sys.stdout = os.fdopen (sys.stdout.fileno (), 'w', 0) 
sys.stderr = os.fdopen (sys.stderr.fileno (), 'w', 0) 
numlabels = int(sys.argv[1])
numparagraphs = int(sys.argv[2])

start = time.time ()

skip = 0
keep = 0
exnum = 0

with open('docid2label','wb') as f:
  for docid, paragraphs in DocGenerator.docs ('text/AA/wiki_00.shuf.bz2'):
    goodparagraphs = [n for n in range (len (paragraphs))
                        if len (paragraphs[n].split ()) > 20]
  
    if len (goodparagraphs) < numparagraphs:
      skip += 1
      continue
  
    keep += 1
  
    f.write ('%s\t%u\n'% (docid, keep))
  
    random.shuffle (goodparagraphs)
  
    for n in goodparagraphs[0:(numparagraphs-1)]:
      tokens = [ t.strip (string.punctuation).translate(None,":|") for t in paragraphs[n].split () ]
      sys.stdout.write ("%u | %s\n"%(keep,' '.join(tokens)))
  
    for n in goodparagraphs[(numparagraphs-1):numparagraphs]:
      tokens = [ t.strip (string.punctuation).translate(None,":|") for t in paragraphs[n].split () ]
      sys.stderr.write ("%u | %s\n"%(keep,' '.join(tokens)))
  
    if keep >= numlabels:
      break

if keep < numlabels:
    exit(1)
