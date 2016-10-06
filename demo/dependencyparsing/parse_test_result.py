from sys import argv
from sys import exit
dict = {}
if len(argv) <4:
	print "Usage: test_conll_file annotation_file_from_vw tag_id_mapping"
	exit(1)
for data in open(argv[3]).readlines():
	dict[data.strip().split()[1]] = data.strip().split()[0]
annotation  = open(argv[2]).readlines()
#for item in list(annotation):
#	if item == ' w\n':
#		annotation.remove(item)
for idx, line in enumerate(open(argv[1]).readlines()):
	item = line.split()
	# conll07
	if len(item) ==10:
		item[-4] = annotation[idx].strip().split(":")[0]
		item[-3] = dict[annotation[idx].strip().split(":")[1]]
	# wsj corpus
	elif len(item) >0:
#		print idx
		item[-2] = annotation[idx].strip().split(":")[0]
		item[-1] = dict[annotation[idx].strip().split(":")[1]]
	print "\t".join(item)

