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
		#if not line.strip():
		#	end_table = True
		#if linenumber >= 9 and (not end_table):
		vw_progress_pattern = '\d+\.\d+\s+\d+\.\d+\s+\d+\s+\d+\.\d+\s+[a-zA-Z0-9]+\s+[a-zA-Z0-9]+\s+\d+'
		matchobj = re.match(vw_progress_pattern, line)

		if matchobj:
			items = line.split()
			avg_loss.append(float(items[0]))
			last_loss.append(float(items[1]))
			wt.append(float(items[3]))
		linenumber += 1

	f.close()

	if len(avg_loss) == 0:
		avg_loss = [0]
		last_loss = [0]
		wt = [0]

	return avg_loss, last_loss, wt

def execute_vw(mod):

	alg_option = ' '
	if mod.adf_on:
		alg_option += ' --cb_explore_adf '
	else:
		alg_option += ' --cb_explore ' + str(mod.num_classes) + ' '

	if mod.cover_on:
		alg_option += ' --cover 5 --psi 0.01 --nounif '
		mod.cb_type = 'dr'
	if mod.epsilon_on:
		alg_option += ' --epsilon ' + str(mod.epsilon) + ' '
	if mod.no_bandit:
		alg_option += ' --no_bandit '
	if mod.no_supervised:
		alg_option += ' --no_supervised '
	#if mod.no_exploration:
	#	alg_option += ' --epsilon 0.0 '
	#if mod.cb_type == 'mtr':
	#	mod.adf_on = True;



	# using two datasets
	#cmd_catfile = '( head -n ' + str(mod.warm_start) + ' ' + mod.dataset_supervised + ';' + ' head -n ' + str(mod.bandit) + ' ' + mod.dataset_bandit + '; )'
	# using only one dataset
	#cmd_catfile = '( head -n ' + str(mod.warm_start + mod.bandit) + ' ' + mod.dataset + '; )'
	#cmd_catfile = '( cat ' + mod.ds_path+mod.dataset + '; )'

	cmd_vw = mod.vw_path + ' --cbify ' + str(mod.num_classes) + ' --cb_type ' + str(mod.cb_type) + ' --warm_start ' + str(mod.warm_start) + ' --bandit ' + str(mod.bandit) + ' --choices_lambda ' + str(mod.choices_lambda) + alg_option + ' --progress ' + str(mod.progress) \
	 + ' -d ' + mod.ds_path + mod.dataset \
	 + ' --corrupt_type_supervised ' + str(mod.corrupt_type_supervised) \
	 + ' --corrupt_prob_supervised ' + str(mod.corrupt_prob_supervised) \
	 + ' --corrupt_type_bandit ' + str(mod.corrupt_type_bandit) \
	 + ' --corrupt_prob_bandit ' + str(mod.corrupt_prob_bandit) \
	 + ' --validation_method ' + str(mod.validation_method) \
	 + ' --weighting_scheme ' + str(mod.weighting_scheme) \
	 + ' --lambda_scheme ' + str(mod.lambda_scheme)

	cmd = cmd_vw
	#cmd = cmd_catfile + ' | ' + cmd_vw

	print cmd

	f = open(mod.vw_output_filename, 'w')
	process = subprocess.Popen(cmd, shell=True, stdout=f, stderr=f)
	#subprocess.check_call(cmd, shell=True)
	process.wait()
	f.close()

def plot_errors(mod):

	execute_vw(mod)
	avg_loss, last_loss, wt = collect_stats(mod)

	if mod.plot_flat:
		# for supervised only, we simply plot a horizontal line using the last point
		len_avg_loss = len(avg_loss)
		avg_loss = avg_loss[len_avg_loss-1]
		avg_loss = [avg_loss for i in range(len_avg_loss)]

	line = plt.plot(wt, avg_loss, mod.plot_color, label=(mod.plot_label))
	avg_error_value = avg_error(mod)

	return avg_error_value


def gen_comparison_graph(mod):

	mod.num_lines = get_num_lines(mod.ds_path+mod.dataset)
	mod.warm_start = int(math.ceil(mod.warm_start_frac * mod.num_lines))
	mod.bandit = mod.num_lines - mod.warm_start
	mod.progress = int(math.ceil(float(mod.bandit) / float(mod.num_checkpoints)))
	mod.num_classes = get_num_classes(mod.dataset)

	#config_name = str(mod.dataset) + '_' + str(mod.fprob1)+'_'+str(mod.fprob2)+'_'+str(mod.warm_start)+'_'+str(mod.bandit)+ '_' + str(mod.cb_type) + '_' + str(mod.choices_lambda)

	config_name = str(mod.dataset) + '_'+str(mod.warm_start)+ '_' + str(mod.cb_type)

	# combined approach, epsilon
	mod.choices_lambda = 2
	mod.weighting_scheme = 1
	mod.lambda_scheme = 3
	mod.no_bandit = False
	mod.no_supervised = False
	mod.no_exploration = False
	mod.cover_on = False
	mod.epsilon_on = True
	mod.plot_color = 'r'
	mod.plot_flat = False
	mod.vw_output_filename = mod.results_path+config_name+'zeroone'+'.txt'
	mod.plot_label = 'zeroone only'
	avg_error_comb_1 = plot_errors(mod)

	# combined approach, cover
	# combined approach, per-dataset weighting
	#mod.choices_lambda = 1
	#mod.no_bandit = False
	#mod.no_supervised = False
	#mod.no_exploration = False
	#mod.cover_on = True
	#mod.vw_output_filename = mod.results_path+config_name+'choices_lambda='+str(mod.choices_lambda)+'.txt'

	mod.choices_lambda = 5
	mod.weighting_scheme = 1
	mod.lambda_scheme = 3
	mod.no_bandit = False
	mod.no_supervised = False
	mod.no_exploration = False
	mod.cover_on = False
	mod.epsilon_on = True
	#'Combined approach, lambda=5'
	mod.plot_color = 'm'
	mod.plot_flat = False
	mod.vw_output_filename = mod.results_path+config_name+'central_minimax_zeroone'+'.txt'
	mod.plot_label = 'Central lambda: minimax, forcing zeroone'
	avg_error_comb_2 = plot_errors(mod)


	# bandit only approach
	mod.choices_lambda = 1
	mod.weighting_scheme = 1
	mod.lambda_scheme = 1
	mod.no_bandit = False
	mod.no_supervised = True
	mod.no_exploration = False
	mod.cover_on = False
	mod.epsilon_on = True
	mod.plot_color = 'b'
	mod.plot_flat = False
	mod.vw_output_filename = mod.results_path+config_name+'_no_supervised'+'.txt'
	mod.plot_label = 'Bandit only'
	avg_error_band_only = plot_errors(mod)

	# supervised only approach
	mod.choices_lambda = 1
	mod.weighting_scheme = 1
	mod.lambda_scheme = 1
	mod.no_bandit = True
	mod.no_supervised = False
	mod.no_exploration = False
	mod.cover_on = False
	mod.epsilon_on = True
	mod.plot_color = 'g'
	mod.plot_flat = True
	mod.vw_output_filename = mod.results_path+config_name+'_no_bandit'+'.txt'
	mod.plot_label = 'Supervised only'
	avg_error_sup_only = plot_errors(mod)


	summary_file = open(mod.summary_file_name, 'a')
	summary_file.write(config_name + ' ' + str(avg_error_comb_1) + ' ' + str(avg_error_comb_2) + ' ' + str(avg_error_band_only) + ' ' + str(avg_error_sup_only) + ' ' + str(mod.bandit) + '\n')
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


def get_num_classes(ds):
	did, n_actions = os.path.basename(ds).split('.')[0].split('_')[1:]
	did, n_actions = int(did), int(n_actions)
	return n_actions


def ds_per_task(mod):
	# put dataset name to the last coordinate so that the task workloads tend to be
	# allocated equally
 	config_all = [item for item in product(mod.choices_cb_types, mod.choices_warm_start_frac, mod.dss)]
	config_task = []
	print len(config_all)
	for i in range(len(config_all)):
		if (i % mod.num_tasks == mod.task_id):
			config_task.append(config_all[i])
			print config_all[i]

	return config_task

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

	errs = rgx.findall(vw_output_text)
	if not errs:
		avge = 0
	else:
		avge = float(errs[0])

	vw_output.close()
	return avge


def main_loop(mod):
	mod.summary_file_name = mod.results_path+str(mod.task_id)+'of'+str(mod.num_tasks)+'.sum'
	summary_file = open(mod.summary_file_name, 'w')
	summary_file.write('dataset' + ' ' + 'zeroone_only' + ' ' + 'central_minimax_zeroone' + ' ' + 'bandit_only' + ' ' + 'supervised_only' + ' ' + 'size' + '\n')
	summary_file.close()

	for mod.cb_type, mod.warm_start_frac, mod.dataset in mod.config_task:
		gen_comparison_graph(mod)


if __name__ == '__main__':
	parser = argparse.ArgumentParser(description='vw job')
	parser.add_argument('task_id', type=int, help='task ID, between 0 and num_tasks - 1')
	parser.add_argument('num_tasks', type=int)
	parser.add_argument('--results_dir', default='../../../figs/')
	parser.add_argument('--warm_start_fraction', type=float)
	parser.add_argument('--corrupt_prob_supervised', type=float)
	parser.add_argument('--corrupt_prob_bandit',type=float)


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

	mod.ds_path = '../../../vwshuffled/'
	mod.vw_path = '../vowpalwabbit/vw'
	mod.results_path = args.results_dir

	#DIR_PATTERN = '../results/cbresults_{}/'

	mod.num_checkpoints = 100
	#mod.warm_start = 50
	#mod.bandit = 4096
	#mod.num_classes = 10
	#mod.cb_type = 'mtr'  #'ips'
    #mod.choices_lambda = 10
	#mod.progress = 25
	mod.adf_on = True

	# use fractions instead of absolute numbers

	#mod.choices_warm_start_frac = [0.01 * pow(2, i) for i in range(1)]
	#mod.choices_warm_start_frac = [0.01, 0.03, 0.1, 0.3]
	#mod.choices_warm_start_frac = [0.03]
	mod.choices_warm_start_frac = [args.warm_start_fraction]
	#mod.choices_warm_start = [0.01 * pow(2, i) for i in range(5)]
	#mod.choices_bandit = [0.01 * pow(2, i) for i in range(5)]

	#mod.choices_warm_start = [pow(2,i) for i in range(11)] #put it here in order to plot 2d mesh
	# we are implicitly iterating over the bandit sample sizes
	#choices_fprob1 = [0.1, 0.2, 0.3]
	#choices_fprob2 = [0.1, 0.2, 0.3]
	#choices_cb_types = ['mtr', 'ips']
	#mod.choices_cb_types = ['mtr', 'ips']
	mod.choices_cb_types = ['mtr']
	#choices_choices_lambda = [pow(2,i) for i in range(10,11)]
	#mod.choices_choices_lambda = [i for i in range(1,3)]
	#mod.choices_choices_lambda = [i for i in range(1,2)]
	#mod.choices_choices_lambda = [1, 3, 5, 7]
	#[i for i in range(10,11)]
	#mod.corrupt_type_supervised = 2
	#mod.corrupt_prob_supervised = 0.3
	mod.corrupt_type_supervised = 1
	#mod.corrupt_prob_supervised = 0.3
	mod.corrupt_prob_supervised = args.corrupt_prob_supervised

	mod.corrupt_type_bandit = 1
	mod.corrupt_prob_bandit = args.corrupt_prob_bandit

	mod.validation_method = 2
	mod.epsilon = 0.05

	#for correctness test
	#mod.choices_warm_start = [20]
	#choices_fprob1 = [0.1]
	#choices_fprob2 = [0.1]

	mod.dss = ds_files(mod.ds_path)
	#mod.dss = ["ds_223_63.vw.gz"]
	#mod.dss = mod.dss[:5]

	# here, we are generating the task specific parameter settings
	# by first generate all parameter setting and pick every num_tasks of them
	mod.config_task = ds_per_task(mod)

	print 'task ' + str(mod.task_id) + ' of ' + str(mod.num_tasks) + ':'

	#print mod.ds_task

	# we only need to vary the warm start fraction, and there is no need to vary the bandit fraction,
	# as each run of vw automatically accumulates the bandit dataset
	main_loop(mod)
