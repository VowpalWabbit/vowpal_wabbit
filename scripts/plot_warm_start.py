import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import subprocess
import pylab
from itertools import product
import os
import math
import argparse
import time
import glob
import re


class model:
	def __init__(self):
		self.no_bandit = False
		self.no_supervised = False

def collect_stats(mod):

	vw_output_filename = mod.vw_output_filename
	# using progress parameter
	# num_rows = mod.bandit / mod.progress
	#print vw_output_filename

	avg_loss = []
	last_loss = []
	wt = []
	end_table = False

	f = open(vw_output_filename, 'r')
	linenumber = 0
	for line in f:
		if not line.strip():
			end_table = True
		if linenumber >= 9 and (not end_table):
			items = line.split()
			avg_loss.append(float(items[0]))
			last_loss.append(float(items[1]))
			wt.append(float(items[3]))
		linenumber += 1

	return avg_loss, last_loss, wt

def execute_vw(mod):

	alg_option = ' '
	if mod.no_bandit:
		alg_option += ' --no_bandit '
	if mod.no_supervised:
		alg_option += ' --no_supervised '
	if mod.no_exploration:
		alg_option += ' --epsilon 0.0 '
	if mod.cb_type == 'mtr':
		mod.adf_on = True;
	if mod.adf_on:
		alg_option += ' --cb_explore_adf '

	# using two datasets
	#cmd_catfile = '( head -n ' + str(mod.warm_start) + ' ' + mod.dataset_supervised + ';' + ' head -n ' + str(mod.bandit) + ' ' + mod.dataset_bandit + '; )'
	# using only one dataset
	#cmd_catfile = '( head -n ' + str(mod.warm_start + mod.bandit) + ' ' + mod.dataset + '; )'
	#cmd_catfile = '( cat ' + mod.ds_path+mod.dataset + '; )'

	cmd_vw = mod.vw_path + ' --cbify ' + str(mod.num_classes) + ' --cb_type ' + str(mod.cb_type) + ' --warm_start ' + str(mod.warm_start) + ' --bandit ' + str(mod.bandit) + ' --choices_lambda ' + str(mod.choices_lambda) + alg_option + ' --progress ' + str(mod.progress) + ' -d ' + mod.ds_path + mod.dataset

	cmd = cmd_vw
	#cmd = cmd_catfile + ' | ' + cmd_vw

	print cmd

	f = open(mod.vw_output_filename, 'w')
	process = subprocess.Popen(cmd, shell=True, stdout=f, stderr=f)
	#subprocess.check_call(cmd, shell=True)
	process.wait()
	f.close()

def gen_comparison_graph(mod):

	mod.num_lines = get_num_lines(mod.ds_path+mod.dataset)
	mod.warm_start = int(math.floor(mod.warm_start_frac * mod.num_lines))
	mod.bandit = mod.num_lines - mod.warm_start
	mod.progress = int(math.ceil(float(mod.bandit) / float(mod.num_checkpoints)))

	#config_name = str(mod.dataset) + '_' + str(mod.fprob1)+'_'+str(mod.fprob2)+'_'+str(mod.warm_start)+'_'+str(mod.bandit)+ '_' + str(mod.cb_type) + '_' + str(mod.choices_lambda)

	config_name = str(mod.dataset) + '_'+str(mod.warm_start)+ '_' + str(mod.cb_type) + '_' + str(mod.choices_lambda)

	# combined approach
	mod.no_bandit = False
	mod.no_supervised = False
	mod.no_exploration = False
	mod.vw_output_filename = mod.results_path+config_name+'.txt'
	execute_vw(mod)

	avg_loss_comb, last_loss_comb, wt_comb = collect_stats(mod)
	line = plt.plot(wt_comb, avg_loss_comb, 'r', label=('Combined approach, #lambdas=' + str(mod.choices_lambda) ))

	avg_error_comb = avg_error(mod)

	# bandit only approach
	mod.no_bandit = False
	mod.no_supervised = True
	mod.no_exploration = False
	mod.vw_output_filename = mod.results_path+config_name+'_no_supervised'+'.txt'
	execute_vw(mod)

	avg_loss_band_only, last_loss_band_only, wt_band_only = collect_stats(mod)
	line = plt.plot(wt_band_only, avg_loss_band_only, 'b', label='Bandit only')

	avg_error_band_only = avg_error(mod)

	# supervised only approach
	mod.no_bandit = True
	mod.no_supervised = False
	mod.no_exploration = False
	mod.vw_output_filename = mod.results_path+config_name+'_no_bandit'+'.txt'
	execute_vw(mod)

	avg_loss_sup_only, last_loss_sup_only, wt_sup_only = collect_stats(mod)
	# for supervised only, we simply plot a horizontal line using the last point
	len_avg_loss = len(avg_loss_sup_only)
	avg_loss = avg_loss_sup_only[len_avg_loss-1]
	avg_loss_sup_only = [avg_loss for i in range(len_avg_loss)]
	line = plt.plot(wt_sup_only, avg_loss_sup_only, 'g', label='Supervised only')

	avg_error_sup_only = avg_error(mod)

	summary_file = open(mod.results_path+str(mod.task_id)+'of'+str(mod.num_tasks)+'.sum', 'a')
	summary_file.write(config_name + ' ' + str(avg_error_comb) + ' ' + str(avg_error_band_only) + ' ' + str(avg_error_sup_only) + '\n')
	summary_file.close()
	print('')


	pylab.legend()
	pylab.xlabel('#bandit examples')
	pylab.ylabel('Progressive validation error')
	pylab.title(mod.dataset + ' warm_start = ' + str(mod.warm_start) + ' cb_type = ' + mod.cb_type)
	#pylab.title('Source 1 feature flipping prob = ' + str(mod.fprob1) + '; source 2 feature flipping prob = ' + str(mod.fprob2) + 'cb_type = '+ mod.cb_type )
	pylab.savefig(mod.results_path+config_name +'.png')
	plt.gcf().clear()

	#plt.show()


def ds_files(ds_path):
	prevdir = os.getcwd()
	os.chdir(ds_path)
	dss = sorted(glob.glob('*.vw.gz'))
	os.chdir(prevdir)
	return dss


def ds_per_task(dss, num_tasks, task_id):
	ds_task = []
	for i in range(len(dss)):
		if (i % num_tasks == task_id):
			ds_task.append(dss[i])

	return ds_task

def get_num_lines(dataset_name):
	ps = subprocess.Popen(('zcat', dataset_name), stdout=subprocess.PIPE)
	output = subprocess.check_output(('wc', '-l'), stdin=ps.stdout)
	ps.wait()
	return int(output)

def avg_error(mod):
	#print mod.vw_output_filename
	vw_output = open(mod.vw_output_filename, 'r')
	vw_output_text = vw_output.read()
	#print vw_output_text
	rgx = re.compile('^average loss = (.*)$', flags=re.M)
	return float(rgx.findall(vw_output_text)[0])



if __name__ == '__main__':
	parser = argparse.ArgumentParser(description='vw job')
	parser.add_argument('task_id', type=int, help='task ID, between 0 and num_tasks - 1')
	parser.add_argument('num_tasks', type=int)
	parser.add_argument('--results_dir', default='../../figs/')
	args = parser.parse_args()
	if args.task_id == 0:
		if not os.path.exists(args.results_dir):
			os.makedirs(args.results_dir)
			import stat
			os.chmod(args.results_dir, os.stat(args.results_dir).st_mode | stat.S_IWOTH)
	else:
		while not os.path.exists(args.results_dir):
			time.sleep(1)

	mod = model()
	mod.num_tasks = args.num_tasks
	mod.task_id = args.task_id

	mod.ds_path = '../../vwshuffled/'
	mod.vw_path = '../vowpalwabbit/vw'
	mod.results_path = args.results_dir

	#DIR_PATTERN = '../results/cbresults_{}/'

	mod.num_checkpoints = 100
	#mod.warm_start = 50
	#mod.bandit = 4096
	mod.num_classes = 10
	#mod.cb_type = 'mtr'  #'ips'
    #mod.choices_lambda = 10
	#mod.progress = 25
	mod.adf_on = True

	# use fractions instead of absolute numbers

	mod.choices_warm_start = [0.01 * pow(2, i) for i in range(4,5)]
	#mod.choices_warm_start = [0.01 * pow(2, i) for i in range(5)]
	#mod.choices_bandit = [0.01 * pow(2, i) for i in range(5)]

	#mod.choices_warm_start = [pow(2,i) for i in range(11)] #put it here in order to plot 2d mesh
	# we are implicitly iterating over the bandit sample sizes
	#choices_fprob1 = [0.1, 0.2, 0.3]
	#choices_fprob2 = [0.1, 0.2, 0.3]
	#choices_cb_types = ['mtr', 'ips']
	#choices_cb_types = ['mtr', 'ips']
	choices_cb_types = ['mtr']
	#choices_choices_lambda = [pow(2,i) for i in range(10,11)]
	choices_choices_lambda = [i for i in range(10,11)]

	#for correctness test
	#mod.choices_warm_start = [20]
	#choices_fprob1 = [0.1]
	#choices_fprob2 = [0.1]

	dss = ds_files(mod.ds_path)
	mod.ds_task = ds_per_task(dss, args.num_tasks, args.task_id)

	print 'task ' + str(mod.task_id) + ' of ' + str(mod.num_tasks) + ':'
	print mod.ds_task

	# we only need to vary the warm start fraction, and there is no need to vary the bandit fraction,
	# as each run of vw automatically accumulates the bandit dataset

	for mod.cb_type, mod.choices_lambda, mod.dataset, mod.warm_start_frac in product(choices_cb_types, choices_choices_lambda, mod.ds_task, mod.choices_warm_start):
		#mod.dataset_supervised = './vw_' + str(mod.fprob1) + '_m.vw'
		#mod.dataset_bandit = './source2_' + str(mod.fprob2) + '_m.vw'
		gen_comparison_graph(mod)
