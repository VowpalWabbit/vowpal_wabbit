import bz2
import re
import string

#-------------------------------------------------
# iterate over documents
#-------------------------------------------------

def docs(filename):
  docid=None
  startdocregex=re.compile('<doc id="(\d+)"')

  with bz2.BZ2File(filename, 'rb') as f:
    for line in f:
      if docid is not None:
        if line[:6] == "</doc>":
          yield int(docid), paragraphs
          docid=None
        elif not line.isspace():
          if "\n" in line:
            curpara.append(line.rstrip('\n'))
            paragraphs.append(' '.join(curpara))
            curpara=[]
          else:
            curpara.appand(line)

      if docid is None:
        m=startdocregex.match (line)
        if m is not None:
          docid=m.group(1)
          paragraphs=[]
          curpara=[]
