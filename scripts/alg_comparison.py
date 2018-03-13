import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import pylab
import os
import glob


def sum_files(result_path):
	prevdir = os.getcwd()
	os.chdir(result_path)
	dss = sorted(glob.glob('*.sum'))
	os.chdir(prevdir)
	return dss

def parse_sum_file(sum_filename):
	f = open(sum_filename, 'r')
	line = f.readline()
	num_cols = len(line.split())
	f.seek(0)
	results = [[] for i in range(num_cols)]

	for line in f:
		splitted = line.split()
		for i in range(len(splitted)):
			if (i == 0):
				results[i].append(splitted[i])
			else:
				results[i].append(float(splitted[i]))
	return results


if __name__ == '__main__':
	results_path = '../figs/'
	dss = sum_files(results_path)

	all_results = []
	for i in range(len(dss)):
		result = parse_sum_file(results_path + dss[i])

		if (i == 0):
			all_results = result
		else:
			num_cols = len(result)
			for j in range(num_cols):
				all_results[j] += result[j]

	print all_results



	# compare combined w/ supervised
	plt.plot([0,1],[0,1])
	plt.scatter(all_results[1], all_results[3])
	plt.title('combined vs supervised only')
	pylab.savefig('comb_v_super' +'.png')
	plt.gcf().clear()

	# compare combined w/ bandit
	plt.plot([0,1],[0,1])
	plt.scatter(all_results[1], all_results[2])
	plt.title('combined vs bandit only')
	pylab.savefig('comb_v_bandit' +'.png')
	plt.gcf().clear()
