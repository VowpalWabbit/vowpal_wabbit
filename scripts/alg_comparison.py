import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import pylab
import os
import glob
import pandas as pd
import scipy.stats as stats
from itertools import compress
from math import sqrt

# this part is changable
#alg1 = 'epsilon'
#alg2 = 'cover'
#alg1 = 'choices_lambda_1'
#alg2 = 'choices_lambda_5'
alg1 = 'instance weighting'
alg2 = 'dataset weighting'

def sum_files(result_path):
	prevdir = os.getcwd()
	os.chdir(result_path)
	dss = sorted(glob.glob('*.sum'))
	os.chdir(prevdir)
	return dss

def parse_sum_file(sum_filename):
	f = open(sum_filename, 'r')
	table = pd.read_table(f, sep=' ', header=None, 	names=['dataset',alg1,alg2,'bandit_only','supervised_only','size'],
	                       lineterminator='\n')

	return table

def get_significance(errors_1, errors_2, sizes):
	significance = []
	for i in range(len(errors_1)):
		significance.append( significant(errors_1[i], errors_2[i], sizes[i]) )
	return significance

def significant(err_1, err_2, size):
	z = (err_1 - err_2) / sqrt( (err_1*(1 - err_1) + err_2*(1-err_2)) / size )

	print z

	if (stats.norm.cdf(z) < 0.05) or (stats.norm.cdf(z) > 0.95):
		return True
	else:
		return False

def plot_comparison(errors_1, errors_2, sizes, title, filename):
	print title

	plt.plot([0,1],[0,1])
	significance = get_significance(errors_1, errors_2, sizes)
	results_signi_1 = list(compress(errors_1, significance))
	results_signi_2 = list(compress(errors_2, significance))
	plt.scatter(results_signi_1, results_signi_2, s=18, c='r')

	insignificance = [not b for b in significance]
	results_insigni_1 = list(compress(errors_1, insignificance))
	results_insigni_2 = list(compress(errors_2, insignificance))

	plt.scatter(results_insigni_1, results_insigni_2, s=2, c='k')
	plt.title(title)
	pylab.savefig(filename)
	plt.gcf().clear()



if __name__ == '__main__':
	#results_path = '../../../lambdas/'
	#results_path = '../../../warm_start_frac=0.1/'
	#results_path = '../../../cover_vs_epsilon/'
	#results_path = '../../../corrupt_supervised_type1_0.3/'
	#results_path = '../../../corrupt_supervised_type2_0.3/'
	#results_path = '../../../supervised_validation/'
	results_path = '../../../weighting_schemes/'

	dss = sum_files(results_path)

	all_results = None
	for i in range(len(dss)):
		result = parse_sum_file(results_path + dss[i])
		if (i == 0):
			all_results = result
		else:
			all_results = all_results.append(result)
	print all_results

	#choices_choices_lambda = sorted(all_results['choices_lambda'].unique())
	#grouped = all_results.groupby('choices_lambda')

	#for cl, results_lambda in grouped:
	#results_lambda = all_results[all_results['choices_lambda'] == cl]
	# compare combined w/ supervised

	results_alg1 = all_results[alg1].tolist()
	results_alg2 = all_results[alg2].tolist()
	results_bandit = all_results['bandit_only'].tolist()
	results_supervised = all_results['supervised_only'].tolist()
	dataset_sizes = all_results['size'].tolist()

	# compare combined w/ bandit
	plot_comparison(results_alg1, results_bandit, dataset_sizes, alg1 + ' vs ' + 'bandit only', results_path + alg1 + ' vs ' + 'bandit only' + '.png')
	plot_comparison(results_alg1, results_supervised, dataset_sizes, alg1 + ' vs ' + 'supervised only', results_path + alg1 + ' vs ' + 'supervised only' + '.png')
	plot_comparison(results_alg2, results_bandit, dataset_sizes, alg2 + ' vs ' + 'bandit only', results_path + alg2 + ' vs ' + 'bandit only' + '.png')
	plot_comparison(results_alg2, results_supervised, dataset_sizes, alg2 + ' vs ' + 'supervised only', results_path + alg2 + ' vs ' + 'supervised only' + '.png')
	plot_comparison(results_alg1, results_alg2, dataset_sizes, alg1 + ' vs ' + alg2, results_path+alg1 + ' vs ' + alg2 + '.png')
