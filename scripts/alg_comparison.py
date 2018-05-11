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


def alg_str(alg_name):
	if (alg_name[0] == 0):
		return 'majority_class'
	if (alg_name[0] == 2):
		return 'supervised_underutil_as_bandit'
	if (alg_name[2] == True and alg_name[3] == True):
		return 'no_update'
	if (alg_name[2] == True and alg_name[3] == False):
		return 'bandit_only'
	if (alg_name[2] == False and alg_name[3] == True):
		return 'supervised_only'
	if (alg_name[2] == False and alg_name[3] == False):
		return 'combined_choices_lambda='+str(alg_name[1])

def problem_str(name_problem):
	return 'supervised_corrupt_type='+str(name_problem[0]) \
			+'_supervised_corrupt_prob='+str(name_problem[1]) \
			+'_bandit_supervised_size_ratio='+str(name_problem[2])



def plot_cdf(alg_name, errs):

	print alg_name
	print errs
	print len(errs)

	plt.step(np.sort(errs), np.linspace(0, 1, len(errs), endpoint=False), label=alg_str(alg_name))


	#raw_input("Press Enter to continue...")

def plot_all_cdfs(alg_results, mod):
	#plot all cdfs:
	print 'printing cdfs..'
	i = 0
	for alg_name, errs in alg_results.iteritems():
		plot_cdf(alg_name, errs)

	plt.legend()
	if mod.normalize_type == 1:
		plt.xlim(-0.2,1)
	elif mod.normalize_type == 2:
		plt.xlim(-1,1)
	plt.ylim(0,1)
	plt.savefig(mod.problemdir+'/cdf.png')
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
				plt.savefig(mod.problemdir+'/'+alg_str(alg_names[i])+'_vs_'+alg_str(alg_names[j])+'.png')
				plt.clf()

def init_results(result_table):
	alg_results = {}
	for idx, row in result_table.iterrows():
		alg_name = (row['warm_start_type'], row['choices_lambda'], row['no_supervised'], row['no_bandit'])
		alg_results[alg_name] = []

	alg_results[(0, 0, False, False)] = []
	return alg_results

def normalize_score(unnormalized_result, mod):
	if mod.normalize_type == 1:
		l = get_best_error(mod.best_error_table, mod.name_dataset)
		u = max(unnormalized_result.values())
		return { k : ((v - l) / (u - l + 1e-4)) for k, v in unnormalized_result.iteritems() }
	elif mod.normalize_type == 2:
		l = unnormalized_result[(1, 1, True, False)]
		return { k : ((v - l) / (l + 1e-4)) for k, v in unnormalized_result.iteritems() }

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

def get_maj_error(maj_error_table, name_dataset):
	name = name_dataset[0]
	maj_error_oneline = maj_error_table[maj_error_table['data'] == name]
	maj_error = maj_error_oneline.loc[maj_error_oneline.index[0], 'avg_error']
	return maj_error

#normalized_results[alg_name].append(normalized_errs[i])
#errs = []
#for idx, row in result_table.iterrows():
#	errs.append(row['avg_error'])

def get_unnormalized_results(result_table):
	new_unnormalized_results = {}
	new_size = 0

	i = 0
	for idx, row in result_table.iterrows():
		if i == 0:
			new_size = row['bandit_size']

		if row['bandit_size'] == new_size:
			alg_name = (row['warm_start_type'], row['choices_lambda'], row['no_supervised'], row['no_bandit'])
			new_unnormalized_results[alg_name] = row['avg_error']
		i += 1

	return new_size, new_unnormalized_results

def update_result_dict(results_dict, new_result):
	for k, v in new_result.iteritems():
		results_dict[k].append(v)


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
			result_table = group_dataset

			grouped_by_algorithm = group_dataset.groupby(['warm_start_type', 'choices_lambda', 'no_supervised', 'no_bandit'])

			mod.name_dataset = name_dataset
			#The 'learning_rate' would be the only free degree here now. Taking the
			#min aggregation will give us the 7 algorithms we are evaluating.
			result_table = grouped_by_algorithm.min()
			result_table = result_table.reset_index()

			#print result_table


			#group_dataset.groupby(['choices_lambda','no_supervised',														'no_bandit'])

			#first time - generate names of algorithms considered
			if normalized_results is None:
				sizes = []
				normalized_results = init_results(result_table)
				unnormalized_results = init_results(result_table)

				#print alg_results
				#dummy = input('')

			#in general (including the first time) - record the error rates of all algorithms
			#print result_table

			new_size, new_unnormalized_result = get_unnormalized_results(result_table)
			new_unnormalized_result[(0, 0, False, False)] = get_maj_error(mod.maj_error_table, mod.name_dataset)

			new_normalized_result = normalize_score(new_unnormalized_result, mod)

			update_result_dict(unnormalized_results, new_unnormalized_result)
			update_result_dict(normalized_results, new_normalized_result)
			sizes.append(new_size)

			#print 'sizes:'
			#print len(sizes)
			#for k, v in unnormalized_results.iteritems():
			#	print len(v)

		mod.problemdir = mod.fulldir+problem_str(mod.name_problem)+'/'
		if not os.path.exists(mod.problemdir):
			os.makedirs(mod.problemdir)

		print 'best_errors', mod.best_error_table
		print 'unnormalized_results', unnormalized_results
		print 'normalized_results', normalized_results

		if mod.pair_comp_on is True:
			plot_all_pair_comp(unnormalized_results, sizes, mod)
		if mod.cdf_on is True:
			plot_all_cdfs(normalized_results, mod)

def save_to_hdf(mod):
	print 'saving to hdf..'
	store = pd.HDFStore('store.h5')
	store['result_table'] = mod.all_results
	store.close()

def load_from_hdf(mod):
	print 'reading from hdf..'
	store = pd.HDFStore('store.h5')
	mod.all_results = store['result_table']
	store.close()

def load_from_sum(mod):
	print 'reading directory..'
	dss = sum_files(mod.results_dir)
	print len(dss)

	#print dss[168]

	all_results = None

	print 'reading sum tables..'
	for i in range(len(dss)):
		print 'result file name: ', dss[i]
		result = parse_sum_file(mod.results_dir + dss[i])

		if (i == 0):
			all_results = result
		else:
			all_results = all_results.append(result)

	print all_results
	mod.all_results = all_results


# This is a hack - need to do this systematically in the future
def load_maj_error(mod):
	maj_error_table = parse_sum_file(mod.maj_error_dir)
	return maj_error_table


if __name__ == '__main__':
	parser = argparse.ArgumentParser(description='result summary')
	parser.add_argument('--results_dir', default='../../../figs/')
	parser.add_argument('--filter', default='1')
	parser.add_argument('--plot_subdir', default='expt1/')
	parser.add_argument('--from_hdf', action='store_true')
	parser.add_argument('--normalize_type', type=int)
	args = parser.parse_args()

	mod = model()

	mod.results_dir = args.results_dir
	mod.filter = args.filter
	mod.plot_subdir = args.plot_subdir
	mod.normalize_type = args.normalize_type
	mod.pair_comp_on = False
	mod.cdf_on = True
	mod.maj_error_dir = '../../../figs_maj_errors/0of1.sum'

	mod.fulldir = mod.results_dir + mod.plot_subdir
	if not os.path.exists(mod.fulldir):
		os.makedirs(mod.fulldir)

	#print args.from_hdf
	#raw_input(' ')
	if args.from_hdf is True:
		load_from_hdf(mod)
	else:
		load_from_sum(mod)
		save_to_hdf(mod)

	#first group by corruption mode, then corruption prob
	#then group by warm start - bandit ratio
	#these constitutes all the problem settings we are looking at (corresponding
	#to each cdf graph)
	all_results = mod.all_results

	mod.best_error_table = all_results[all_results['choices_lambda'] == 0]
	all_results = all_results[all_results['choices_lambda'] != 0]

	#ignore the no update row:
	all_results = all_results[(all_results['no_supervised'] == False) | (all_results['no_bandit'] == False)]

	#filter choices_lambdas = 2,4,8?
	#if (alg_name[2] == False and alg_name[3] == False and alg_name[1] != 8):
	#	pass
	#else:

	mod.maj_error_table = load_maj_error(mod)

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
