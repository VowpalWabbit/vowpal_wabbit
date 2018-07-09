import random
import numpy as np

classes = 2
m = 10
kwperclass = 2

def gen_keyword():
	keyword = np.zeros((classes, m))

	for i in range(classes):
		shuffled = range(m)
		random.shuffle(shuffled)

		for j in range(kwperclass):
			keyword[i,shuffled[j]] = 1

	return keyword


def classify(classifier, example):
		result = classifier.dot(example)
		return np.argmax(result)

def gen_datasets(filename, keyword, num_samples, fprob):

	f = open(filename+".vw", "w")
	g = open(filename+"_m.vw", "w")

	for i in range(num_samples):
		c = random.randint(0, classes-1)

		#generate a pair of datasets (one is cost-sensitive, the other is multiclass)
		for l in range(classes):
			f.write(str(l+1)+':')
			cost = 1
			if l == c:
				cost = 0
			f.write(str(cost)+' ')

		g.write(str(c+1))

		f.write(' | ')
		g.write(' | ')

		vec = np.zeros(m)

		for j in range(m):
			flip = np.random.choice([False,True],p=[1-fprob, fprob])
			if flip:
				vec[j] = 2 * (1-keyword[c][j]) - 1
			else:
				vec[j] = 2 * keyword[c][j] - 1

		for j in range(m):
			f.write('w'+str(j)+':')
			f.write(str(vec[j])+' ')
			g.write('w'+str(j)+':')
			g.write(str(vec[j])+' ')

		#print 'Is the prediction equal to the class label? ', classify(keyword, vec) == c
		f.write('\n')
		g.write('\n')

	f.close()
	g.close()



if __name__ == '__main__':

	keyword = gen_keyword()
	# Remember to generate a pair of datasets at the same time
	# so that the class-dependent feature is retained


	num_samples = 10000
	fprob = 0.1
	filename = "source1"+'_'+str(fprob)

	gen_datasets(filename, keyword, num_samples, fprob)


	num_samples = 10000
	fprob = 0.1
	filename = "source2"+'_'+str(fprob)

	gen_datasets(filename, keyword, num_samples, fprob)
