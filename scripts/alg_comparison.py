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
import argparse
import numpy as np


class model:
	def __init__(self):
		pass

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
	#f.seek(0, 0)
	table = pd.read_table(f, sep='\s+',lineterminator='\n')

	return table

def get_z_scores(errors_1, errors_2, sizes):
	z_scores = []
	for i in range(len(errors_1)):
		#print i
		z_scores.append( z_score(errors_1[i], errors_2[i], sizes[i]) )
	return z_scores

def z_score(err_1, err_2, size):
	if (abs(err_1) < 1e-6 or abs(err_1) > 1-1e-6) and (abs(err_2) < 1e-6 or abs(err_2) > 1-1e-6):
		return 0

	#print err_1, err_2, size, sqrt( (err_1*(1 - err_1) + err_2*(1-err_2)) / size )

	z = (err_1 - err_2) / sqrt( (err_1*(1 - err_1) + err_2*(1-err_2)) / size )
	return z
	#print z

def is_significant(z):
	if (stats.norm.cdf(z) < 0.05) or (stats.norm.cdf(z) > 0.95):
		return True
	else:
		return False

def plot_comparison(errors_1, errors_2, sizes):
	#print title
	plt.plot([0,1],[0,1])
	z_scores = get_z_scores(errors_1, errors_2, sizes)
	sorted_z_scores = sorted(enumerate(z_scores), key=lambda x:x[1])
	#for s in sorted_z_scores:
	#	print s, is_significant(s[1])

	significance = map(is_significant, z_scores)
	results_signi_1 = list(compress(errors_1, significance))
	results_signi_2 = list(compress(errors_2, significance))
	plt.scatter(results_signi_1, results_signi_2, s=18, c='r')

	insignificance = [not b for b in significance]
	results_insigni_1 = list(compress(errors_1, insignificance))
	results_insigni_2 = list(compress(errors_2, insignificance))

	plt.scatter(results_insigni_1, results_insigni_2, s=2, c='k')


def normalized_score(lst, l):
	#print lst
	#l = min(lst)
	u = max(lst)
	return [ (item - l) / (u - l + 1e-4) for item in lst ]

def alg_str(alg_name):
	if (alg_name[1] == True and alg_name[2] == True):
		return 'no_update'
	if (alg_name[1] == True and alg_name[2] == False):
		return 'bandit_only'
	if (alg_name[1] == False and alg_name[2] == True):
		return 'supervised_only'
	if (alg_name[1] == False and alg_name[2] == False):
		return 'combined_choices_lambda='+str(alg_name[0])

def problem_str(name_problem):
	return 'supervised_corrupt_type='+str(name_problem[0]) \
			+'_supervised_corrupt_prob='+str(name_problem[1]) \
			+'_bandit_supervised_size_ratio='+str(name_problem[2])



def plot_cdf(alg_name, errs):

	plt.step(np.sort(errs), np.linspace(0, 1, len(errs), endpoint=False), label=alg_str(alg_name))

	print alg_name
	print errs
	print len(errs)
	#raw_input("Press Enter to continue...")

def plot_all_cdfs(alg_results, mod):
	#plot all cdfs:
	i = 0
	for alg_name, errs in alg_results.iteritems():
		plot_cdf(alg_name, errs)

	plt.legend()
	plt.xlim(-1,1)
	plt.ylim(0,1)
	plt.savefig(mod.fulldir+problem_str(mod.name_problem)+'.png')
	plt.clf()


def plot_all_pair_comp(alg_results, sizes, mod):
	alg_names = alg_results.keys()

	for i in range(len(alg_names)):
		for j in range(len(alg_names)):
			if i < j:
				errs_1 = alg_results[alg_names[i]]
				errs_2 = alg_results[alg_names[j]]

				print len(errs_1), len(errs_2), len(sizes)
				#raw_input('Press any key to continue..')

				plot_comparison(errs_1, errs_2, sizes)

				plt.title(alg_str(alg_names[i])+' vs '+alg_str(alg_names[j]))
				plt.savefig(mod.fulldir+problem_str(mod.name_problem)+'_'+alg_str(alg_names[i])+'_vs_'+alg_str(alg_names[j])+'.png')
				plt.clf()

def init_results(result_table):
	alg_results = {}
	for idx, row in result_table.iterrows():
		alg_name = (row['choices_lambda'], row['no_supervised'], row['no_bandit'])
		alg_results[alg_name] = []
	return alg_results

def get_best_error(best_error_table, name_dataset):
	name = name_dataset[0]
	best_error_oneline = best_error_table[best_error_table['dataset'] == name]
	best_error = best_error_oneline.loc[best_error_oneline.index[0], 'avg_error']
	#print name
	#raw_input("...")
	#print best_error_oneline
	#raw_input("...")
	#print best_error
	#raw_input("...")
	return best_error



def plot_all(mod, all_results):
	grouped_by_problem = all_results.groupby(['corrupt_type_supervised',
						'corrupt_prob_supervised','bandit_supervised_size_ratio'])

	#then group by dataset and warm_start size (corresponding to each point in cdf)
	for name_problem, group_problem in grouped_by_problem:
		normalized_results = None
		unnormalized_results = None
		sizes = None
		mod.name_problem = name_problem

		grouped_by_dataset = group_problem.groupby(['dataset','warm_start_size'])
		#then select unique combinations of (no_supervised, no_bandit, choices_lambda)
		#e.g. (True, True, 1), (True, False, 1), (False, True, 1), (False, False, 2)
		#(False, False, 8), and compute a normalized score

		for name_dataset, group_dataset in grouped_by_dataset:
			result_table = group_dataset #group_dataset.groupby(['choices_lambda','no_supervised',														'no_bandit'])

			#first time - generate names of algorithms considered
			if normalized_results is None:
				sizes = []
				normalized_results = init_results(result_table)
				unnormalized_results = init_results(result_table)

				#print alg_results
				#dummy = input('')

			#in general (including the first time) - record the error rates of all algorithms

			err_best = get_best_error(mod.best_error_table, name_dataset)
			errs = []
			for idx, row in result_table.iterrows():
				errs.append(row['avg_error'])
			normalized_errs = normalized_score(errs, err_best)

			i = 0
			for idx, row in result_table.iterrows():
				if i == 0:
					sizes.append(row['total_size'])
				alg_name = (row['choices_lambda'], row['no_supervised'], row['no_bandit'])
				unnormalized_results[alg_name].append(errs[i])
				normalized_results[alg_name].append(normalized_errs[i])
				i += 1

		plot_all_pair_comp(unnormalized_results, sizes, mod)
		plot_all_cdfs(normalized_results, mod)






if __name__ == '__main__':
	parser = argparse.ArgumentParser(description='result summary')
	parser.add_argument('--results_dir', default='../../../figs/')
	parser.add_argument('--filter', default='1')
	parser.add_argument('--plot_subdir', default='expt1/')
	args = parser.parse_args()

	mod = model()

	mod.results_dir = args.results_dir
	mod.filter = args.filter
	mod.plot_subdir = args.plot_subdir

	mod.fulldir = mod.results_dir + mod.plot_subdir
	if not os.path.exists(mod.fulldir):
		os.makedirs(mod.fulldir)

	#results_dir = '../../../lambdas/'
	#results_dir = '../../../warm_start_frac=0.1/'
	#results_dir = '../../../cover_vs_epsilon/'
	#results_dir = '../../../corrupt_supervised_type1_0.3/'
	#results_dir = '../../../expt_0403/corrupt_supervised_type2_0.3/'
	#results_dir = '../../../expt_0403/supervised_validation/'
	#results_dir = '../../../weighting_schemes/'
	#results_dir = '../../../central_lambda/'
	#results_dir = '../../../central_lambda_naive/'
	#results_dir = '../../../central_lambda_zeroone/'
	#results_dir = '../../../type2_0.3/'
	#results_dir = '../../../type1_0.3/'
	#results_dir = '../../../type2_1/'
	#results_dir = '../../../type2_0.65/'
	#results_dir = '../../../type2_0.3/'

	dss = sum_files(mod.results_dir)

	#print dss[168]

	all_results = None

	for i in range(len(dss)):
		print 'result file name: ', dss[i]
		result = parse_sum_file(mod.results_dir + dss[i])

		if (i == 0):
			all_results = result
		else:
			all_results = all_results.append(result)

	print all_results

	#first group by corruption mode, then corruption prob
	#then group by warm start - bandit ratio
	#these constitutes all the problem settings we are looking at (corresponding
	#to each cdf graph)

	mod.best_error_table = all_results[all_results['choices_lambda'] == 0]
	all_results = all_results[all_results['choices_lambda'] != 0]

	if mod.filter == '1':
		pass
	elif mod.filter == '2':
		#print all_results['warm_start_size'] >= 100
		#raw_input(' ')
		all_results = all_results[all_results['warm_start_size'] >= 100]
	elif mod.filter == '3':
		all_results = all_results[all_results['num_classes'] >= 3]
	elif mod.filter == '4':
		all_results = all_results[all_results['num_classes'] <= 2]

	plot_all(mod, all_results)

	#if i >= 331 and i <= 340:
	#	print 'result:', result
	#	print 'all_results:', all_results


	#result = parse_sum_file(results_dir + '400of600.sum')
	#print result

	#choices_choices_lambda = sorted(all_results['choices_lambda'].unique())
	#grouped = all_results.groupby('choices_lambda')

	#for cl, results_lambda in grouped:
	#results_lambda = all_results[all_results['choices_lambda'] == cl]
	# compare combined w/ supervised
	'''
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
	'''
	#print alg1
	#print results_alg1

	# compare combined w/ bandit
	#plot_comparison(results_alg1, results_bandit, dataset_sizes, alg1 + ' vs ' + 'bandit only', results_dir + alg1 + ' vs ' + 'bandit only' + '.png')
	#plot_comparison(results_alg1, results_supervised, dataset_sizes, alg1 + ' vs ' + 'supervised only', results_dir + alg1 + ' vs ' + 'supervised only' + '.png')
	#plot_comparison(results_alg2, results_bandit, dataset_sizes, alg2 + ' vs ' + 'bandit only', results_dir + alg2 + ' vs ' + 'bandit only' + '.png')
	#plot_comparison(results_alg2, results_supervised, dataset_sizes, alg2 + ' vs ' + 'supervised only', results_dir + alg2 + ' vs ' + 'supervised only' + '.png')
	#plot_comparison(results_alg1, results_alg2, dataset_sizes, alg1 + ' vs ' + alg2, results_dir+alg1 + ' vs ' + alg2 + '.png')
