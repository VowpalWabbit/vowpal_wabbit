import random
import numpy as np

classes = 10
m = 100

def gen_keyword():

	kwperclass = 20

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



if __name__ == '__main__':


	filename = "text_lownoise"

	f = open(filename+".vw", "w")
	g = open(filename+"_m.vw", "w")

	keyword = gen_keyword()	


	samples = 10000
	fprob = 0

	cs = False

	for i in range(samples):
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
		
		



