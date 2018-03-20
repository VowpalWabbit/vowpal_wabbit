import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import pylab
import os
import glob
import pandas as pd


def sum_files(result_path):
	prevdir = os.getcwd()
	os.chdir(result_path)
	dss = sorted(glob.glob('*.sum'))
	os.chdir(prevdir)
	return dss

def parse_sum_file(sum_filename):
	f = open(sum_filename, 'r')
	table = pd.read_table(f, sep=' ', header=None, names=['dataset','combined','bandit_only','supervised_only','choices_lambda'],
                       lineterminator='\n')
	return table


if __name__ == '__main__':
	results_path = '../../../figs/'
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
	grouped = all_results.groupby('choices_lambda')

	for cl, results_lambda in grouped:
		#results_lambda = all_results[all_results['choices_lambda'] == cl]
		# compare combined w/ supervised
		results_combined = results_lambda['combined'].tolist()
		results_bandit = results_lambda['bandit_only'].tolist()
		results_supervised = results_lambda['supervised_only'].tolist()

		# compare combined w/ bandit
		plt.plot([0,1],[0,1])
		plt.scatter(results_combined, results_bandit)
		plt.title('combined vs bandit only')
		pylab.savefig('comb_v_bandit ' + 'choices_lambda=' + str(cl) +'.png')
		plt.gcf().clear()

		plt.plot([0,1],[0,1])
		plt.scatter(results_combined, results_supervised)
		plt.title('combined vs supervised only')
		pylab.savefig('comb_v_supervised ' + 'choices_lambda=' + str(cl) +'.png')
		plt.gcf().clear()
