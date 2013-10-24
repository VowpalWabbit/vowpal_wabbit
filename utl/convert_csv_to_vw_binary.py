#!/usr/bin/python
# 2013 Eric Whyne http://www.datamungeblog.com/
import re
import sys

if len(sys.argv) != 6:
    sys.exit('This script converts csv data to a very basic vowpal wabbit binary classifier training data format.\n\nUsage: %s <infile.csv> <outfile.vw> <category index> <positive regex> <negative regex>\n\nCategory index and the regular expression options define where to look for the binary category and how to identify it.' % sys.argv[0])

infile = open (sys.argv[1],'r')
outfile = open (sys.argv[2],'w')
category_index = int(sys.argv[3])
pregex = sys.argv[4]
nregex = sys.argv[5]

for line in infile:
  data = line.split(',')
  category = data.pop(category_index)
  if re.search(nregex, category): # regex for negative category
    category = "-1"
  elif re.search(pregex, category): # regex for positive category
    category = '1'
  else:
    print "Regex did not mach record", line
    exit()
  outline = category + " | "
  colnum = 0
  for col in data:
    colstr = str(colnum)
    col = re.sub(r'\s','',col) # remove all whitespace
    if re.search('^[0-9.]*$', col): # test if it's a number
      outline = outline + "f" + colstr + ":" + col + " "
    else:
      outline = outline + col + ' '
    colnum = colnum + 1
  outline = outline + "\n"
  outfile.write(outline)
     
