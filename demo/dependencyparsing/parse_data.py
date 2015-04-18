from sys import argv

hash = {}
def readtags():
	for line in open('tags').readlines():
		hash[line.split()[0]] = int(line.strip().split()[1])

if __name__ == '__main__':
	c = 1
	readtags()
	if len(argv) != 3:
		print 'parseDepData.py input output'
	data = open(argv[1]).readlines()
	writer = open(argv[2],'w')
	for line in data:
		if line == '\n':
			writer.write('\n')
			continue
		splits = line.strip().lower().split()
		strw = "|w %s"%splits[1].replace(":","COL");
		strp = "|p %s"%splits[4].replace(":","COL");				
		tag = splits[8]
		if tag not in hash:
			hash[tag] = c
			c+=1
		#writer.write('%s 1.0  %s:%s%s %s\n'%((int(splits[7])+1) + (hash[tag]<<8), int(splits[7]),tag,strw, strp))
		writer.write('%s %s %s:%s%s %s\n' % (int(splits[7]), hash[tag], int(splits[7]), tag, strw, strp))
	writer.close()

