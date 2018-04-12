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
#alg1 = 'instance weighting'
#alg2 = 'dataset weighting'

def sum_files(result_path):
	prevdir = os.getcwd()
	os.chdir(result_path)
	dss = sorted(glob.glob('*.sum'))
	os.chdir(prevdir)
	return dss

def parse_sum_file(sum_filename):
	f = open(sum_filename, 'r')
	table = pd.read_table(f, sep=' ',lineterminator='\n')

	return table

def get_z_scores(errors_1, errors_2, sizes):
	z_scores = []
	for i in range(len(errors_1)):
		z_scores.append( z_score(errors_1[i], errors_2[i], sizes[i]) )
	return z_scores

def z_score(err_1, err_2, size):
	z = (err_1 - err_2) / sqrt( (err_1*(1 - err_1) + err_2*(1-err_2)) / size )
	return z
	#print z

def is_significant(z):
	if (stats.norm.cdf(z) < 0.05) or (stats.norm.cdf(z) > 0.95):
		return True
	else:
		return False

def plot_comparison(errors_1, errors_2, sizes, title, filename):
	print title

	plt.plot([0,1],[0,1])
	z_scores = get_z_scores(errors_1, errors_2, sizes)
	sorted_z_scores = sorted(enumerate(z_scores), key=lambda x:x[1])
	for s in sorted_z_scores:
		print s, is_significant(s[1])

	significance = map(is_significant, z_scores)
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
	#results_path = '../../../expt_0403/corrupt_supervised_type2_0.3/'
	#results_path = '../../../expt_0403/supervised_validation/'
	#results_path = '../../../weighting_schemes/'
	#results_path = '../../../central_lambda/'
	#results_path = '../../../central_lambda_naive/'
	#results_path = '../../../central_lambda_zeroone/'
	#results_path = '../../../type2_0.3/'
	#results_path = '../../../type1_0.3/'
	#results_path = '../../../type2_1/'
	#results_path = '../../../type2_0.65/'
	results_path = '../../../type2_0.3/'

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
	alg1 = all_results.columns[1]
	alg2 = all_results.columns[2]
	bandit_only = all_results.columns[3]
	supervised_only = all_results.columns[4]
	sizes = all_results.columns[5]

	results_alg1 = all_results[alg1].tolist()
	results_alg2 = all_results[alg2].tolist()
	results_bandit = all_results[bandit_only].tolist()
	results_supervised = all_results[supervised_only].tolist()
	dataset_sizes = all_results[sizes].tolist()

	print alg1
	print results_alg1

	# compare combined w/ bandit
	plot_comparison(results_alg1, results_bandit, dataset_sizes, alg1 + ' vs ' + 'bandit only', results_path + alg1 + ' vs ' + 'bandit only' + '.png')
	plot_comparison(results_alg1, results_supervised, dataset_sizes, alg1 + ' vs ' + 'supervised only', results_path + alg1 + ' vs ' + 'supervised only' + '.png')
	plot_comparison(results_alg2, results_bandit, dataset_sizes, alg2 + ' vs ' + 'bandit only', results_path + alg2 + ' vs ' + 'bandit only' + '.png')
	plot_comparison(results_alg2, results_supervised, dataset_sizes, alg2 + ' vs ' + 'supervised only', results_path + alg2 + ' vs ' + 'supervised only' + '.png')
	plot_comparison(results_alg1, results_alg2, dataset_sizes, alg1 + ' vs ' + alg2, results_path+alg1 + ' vs ' + alg2 + '.png')
