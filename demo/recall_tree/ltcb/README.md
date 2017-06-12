Recall tree demo for ltcb
-------------------------------

There are two versions of the LTCB: enwik8 is the small one, and 
enwik9 is the big one.  http://mattmahoney.net/dc/textdata.html

  * make enwik8.train.vw.gz: makes the following files
     1. enwik8.zip: downloaded
     2. enwik8.freq: base frequency of each token in descending order
       2a. labels are the first 80K of these
     3. enwik8.train.vw.gz: training set
     4. enwik8.test.vw.gz: test set
  * make oaa8: example invocation of VW on oaa for enwik8
  * make enwik9.train.vw.gz: same files but larger (number of examples)
  * make oaa9: example invocation of VW on oaa for enwik9


the VW data files consist of the label and the 10 preceeding words,
no boundary modeling, i.e., 11th word is first example generated,
and there is no attempt to predict ``end of document''

infrequent words are replaced with unknown in the labels, so label 
80001 is the unknown label.  however all words are preseved in input.
