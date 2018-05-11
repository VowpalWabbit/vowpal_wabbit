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
	avg_error_value = avg_error(mod)
	actual_var_value = actual_var(mod)
	ideal_var_value = ideal_var(mod)

	vw_run_results = []
	vw_result_template = {
	'bandit_size': 0,
	'bandit_supervised_size_ratio': 0,
	'avg_error': 0.0,
	'actual_variance': 0.0,
	'ideal_variance': 0.0
	}

	if mod.compute_optimal is True:
		vw_result = vw_result_template.copy()
		if 'optimal_approx' in mod.param:
			# this condition is for computing the optimal error
			vw_result['avg_error'] = avg_error_value
		else:
			# this condition is for computing the majority error
			err =  1 - float(mod.result['majority_size']) / mod.result['total_size']
			vw_result['avg_error'] = float('%0.5f' % err)
		vw_run_results.append(vw_result)
		return vw_run_results

	f = open(mod.vw_output_filename, 'r')

	i = 0
	for line in f:
		vw_progress_pattern = '\d+\.\d+\s+\d+\.\d+\s+\d+\s+\d+\.\d+\s+[a-zA-Z0-9]+\s+[a-zA-Z0-9]+\s+\d+.*'
		matchobj = re.match(vw_progress_pattern, line)

		if matchobj:
			s = line.split()
			if len(s) >= 8:
				s = s[:7]
			avg_loss_str, last_loss_str, counter_str, weight_str, curr_label_str, \
			curr_pred_str, curr_feat_str = s

			avg_loss = float(avg_loss_str)
			bandit_effective = int(float(weight_str))

			for ratio in mod.critical_size_ratios:
				if bandit_effective >= (1 - 1e-7) * mod.result['warm_start'] * ratio and \
				bandit_effective <= (1 + 1e-7) * mod.result['warm_start'] * ratio:
					vw_result = vw_result_template.copy()
					vw_result['bandit_size'] = bandit_effective
					vw_result['bandit_supervised_size_ratio'] = ratio
					vw_result['avg_error'] = avg_loss
					vw_result['actual_variance'] = actual_var_value
					vw_result['ideal_variance'] = ideal_var_value
					vw_run_results.append(vw_result)
	f.close()
	return vw_run_results


def gen_vw_options_list(vw_options):
	vw_options_list = []
	for k, v in vw_options.iteritems():
		vw_options_list.append('--'+str(k))
		vw_options_list.append(str(v))
	return vw_options_list

def gen_vw_options(mod):
	vw_options = {}
	vw_options['data'] = mod.data_full_path
	vw_options['progress'] = mod.result['progress']

	if 'optimal_approx' in mod.param:
		vw_options['passes'] = 5
		vw_options['oaa'] = mod.result['num_classes']
		vw_options['cache_file'] = mod.data_full_path + '.cache'
	elif 'majority_approx' in mod.param:
		# basically we would like to skip vw running as fast as possible
		vw_options['cbify'] = mod.result['num_classes']
		vw_options['warm_start'] = 0
		vw_options['bandit'] = 0
	else:
		vw_options['corrupt_type_bandit'] = mod.corrupt_type_bandit
		vw_options['corrupt_prob_bandit'] = mod.corrupt_prob_bandit
		vw_options['bandit'] = mod.bandit

		if mod.adf_on is True:
			vw_options['cb_explore_adf'] = ' '
		else:
			vw_options['cb_explore'] = mod.num_classes

		if mod.epsilon_on is True:
			vw_options['epsilon'] = mod.epsilon

		vw_options['cb_type'] = mod.param['cb_type']
		vw_options['choices_lambda'] = mod.param['choices_lambda']
		vw_options['corrupt_type_supervised'] = mod.param['corrupt_type_supervised']
		vw_options['corrupt_prob_supervised'] = mod.param['corrupt_prob_supervised']
		vw_options['lambda_scheme'] = mod.param['lambda_scheme']
		if mod.param['no_supervised'] is True:
			vw_options['no_supervised'] = ' '
		if mod.param['no_bandit'] is True:
			vw_options['no_bandit'] = ' '
		vw_options['learning_rate'] = mod.param['learning_rate']
		vw_options['warm_start_type'] = mod.param['warm_start_type']

		vw_options['cbify'] = mod.result['num_classes']
		vw_options['warm_start'] = mod.result['warm_start']
		vw_options['overwrite_label'] = mod.result['majority_class']
		vw_options['validation_method'] = mod.result['validation_method']
		vw_options['weighting_scheme'] = mod.result['weighting_scheme']

		#if mod.cover_on:
		#	alg_option += ' --cover 5 --psi 0.01 --nounif '
			#mod.cb_type = 'dr'
	return vw_options

def execute_vw(mod):
	vw_options = gen_vw_options(mod)
	vw_options_list = gen_vw_options_list(vw_options)
	cmd = disperse([mod.vw_path]+vw_options_list, ' ')
	print cmd

	f = open(mod.vw_output_filename, 'w')
	process = subprocess.Popen(cmd, shell=True, stdout=f, stderr=f)
	#subprocess.check_call(cmd, shell=True)
	process.wait()
	f.close()

def disperse(l, ch):
	s = ''
	for item in l:
		s += str(item)
		s += ch
	return s

def param_to_str(param):
	param_list = [ str(k)+'='+str(v) for k,v in param.iteritems() ]
	return disperse(param_list, ',')

def param_to_result(param, result):
	for k, v in param.iteritems():
		if k in result:
			result[k] = v

def gen_comparison_graph(mod):
	mod.result = mod.result_template.copy()

	if 'majority_approx' in mod.param or 'optimal_approx' in mod.param:
		mod.compute_optimal = True
	else:
		mod.compute_optimal = False

	param_to_result(mod.param, mod.result)
	mod.data_full_path = mod.ds_path + str(mod.param['fold']) + '/' + mod.param['data']

	mod.result['fold'] = mod.param['fold']
	mod.result['total_size'] = get_num_lines(mod.data_full_path)
	mod.result['num_classes'] = get_num_classes(mod.data_full_path)
	mod.result['majority_size'], mod.result['majority_class'] = get_majority_class(mod.data_full_path)
	mod.result['progress'] = int(math.ceil(float(mod.result['total_size']) / float(mod.num_checkpoints)))
	mod.vw_output_dir = mod.results_path + remove_suffix(mod.param['data']) + '/'
	mod.vw_output_filename = mod.vw_output_dir + param_to_str(mod.param) + '.txt'

	if mod.compute_optimal is False:
		mod.result['warm_start'] = mod.param['warm_start_multiplier'] * mod.result['progress']
		mod.bandit = mod.result['total_size'] - mod.result['warm_start']
		mod.result['validation_method'] = mod.validation_method
		mod.result['weighting_scheme'] = mod.weighting_scheme
		mod.result['corrupt_type_bandit'] = mod.corrupt_type_bandit
		mod.result['corrupt_prob_bandit'] = mod.corrupt_prob_bandit
		mod.result['fold'] = mod.param['fold']

	#plot_errors(mod)
	execute_vw(mod)
	vw_run_results = collect_stats(mod)
	for vw_result in vw_run_results:
		result_combined = merge_two_dicts(mod.result, vw_result)
		result_formatted = format_result(mod.result_template, result_combined)
		record_result(mod, result_formatted)

	print('')

def format_result(result_template, result):
	result_formatted = result_template.copy()
	for k, v in result.iteritems():
		result_formatted[k] = v
	return result_formatted

def record_result(mod, result):
	result_row = []
	for k in mod.result_header_list:
		result_row.append(result[k])

	summary_file = open(mod.summary_file_name, 'a')
	summary_file.write( disperse(result_row, '\t') + '\n')
	summary_file.close()

def ds_files(ds_path):
	prevdir = os.getcwd()
	os.chdir(ds_path)
	dss = sorted(glob.glob('*.vw.gz'))
	#dss = [ds_path+ds for ds in dss]
	os.chdir(prevdir)
	return dss

def merge_two_dicts(x, y):
	#print 'x = ', x
	#print 'y = ', y
	z = x.copy()   # start with x's keys and values
	z.update(y)    # modifies z with y's keys and values & returns None
	return z

def param_cartesian(param_set_1, param_set_2):
	prod = []
	for param_1 in param_set_1:
		for param_2 in param_set_2:
			prod.append(merge_two_dicts(param_1, param_2))
	return prod

def param_cartesian_multi(param_sets):
	#print param_sets
	prod = [{}]
	for param_set in param_sets:
		prod = param_cartesian(prod, param_set)
	return prod

def dictify(param_name, param_choices):
	result = []
	for param in param_choices:
		dic = {}
		dic[param_name] = param
		result.append(dic)
	return result

def params_per_task(mod):
	# Problem parameters
	params_corrupt_type_sup = dictify('corrupt_type_supervised', mod.choices_corrupt_type_supervised)
	params_corrupt_prob_sup = dictify('corrupt_prob_supervised', mod.choices_corrupt_prob_supervised)
	params_warm_start_multiplier = dictify('warm_start_multiplier', mod.warm_start_multipliers)
	params_learning_rate = dictify('learning_rate', mod.learning_rates)

	# could potentially induce a bug if the maj and best does not have this parameter
	params_fold = dictify('fold', mod.folds)

	# Algorithm parameters
	params_cb_type = dictify('cb_type', mod.choices_cb_type)

	# Common parameters
	params_common = param_cartesian_multi([params_corrupt_type_sup, params_corrupt_prob_sup, params_warm_start_multiplier, params_learning_rate, params_cb_type, params_fold])
	params_common = filter(lambda param: param['corrupt_type_supervised'] == 1 or abs(param['corrupt_prob_supervised']) > 1e-4, params_common)

	# Baseline parameters construction
	if mod.baselines_on:
		params_baseline_basic = [
		[{'choices_lambda': 1, 'warm_start_type': 1, 'lambda_scheme': 3}], [{'no_supervised': True}, {'no_supervised': False}], [{'no_bandit': True}, {'no_bandit': False}]
		]
		params_baseline = param_cartesian_multi([params_common] + params_baseline_basic)
		params_baseline = filter(lambda param: param['no_supervised'] == True or param['no_bandit'] == True, params_baseline)
	else:
		params_baseline = []


	# Algorithm parameters construction
	if mod.algs_on:
		params_choices_lambd = dictify('choices_lambda', mod.choices_choices_lambda)
		params_algs_1 = param_cartesian(params_choices_lambd, [{'no_supervised': False, 'no_bandit': False, 'warm_start_type': 1, 'lambda_scheme': 3}] )
		params_algs_2 = [{'no_supervised': False, 'no_bandit': False, 'warm_start_type': 2, 'lambda_scheme': 1, 'choices_lambda':1}]
		params_algs = param_cartesian( params_common, params_algs_1 + params_algs_2 )
	else:
		params_algs = []

	# Optimal baselines parameter construction
	if mod.optimal_on:
		params_optimal = [{ 'optimal_approx': True }]
	else:
		params_optimal = []

	if mod.majority_on:
		params_majority = [{ 'majority_approx': True }]
	else:
		params_majority = []

	#print len(params_baseline)
	#print len(params_algs)
	#print len(params_common)
	#raw_input('..')


	# Common factor in all 3 groups: dataset
	params_dataset = dictify('data', mod.dss)
	params_all = param_cartesian( params_dataset, params_baseline + params_algs + params_optimal + params_majority )
	params_all = sorted(params_all)
	print len(params_all)
	for row in params_all:
		print row
	return get_params_task(params_all)


def get_params_task(params_all):
	params_task = []
	for i in range(len(params_all)):
		if (i % mod.num_tasks == mod.task_id):
			params_task.append(params_all[i])
	return params_task

def get_num_lines(dataset_name):
	num_lines = subprocess.check_output(('zcat ' + dataset_name + ' | wc -l'), shell=True)
	return int(num_lines)

def get_num_classes(ds):
	# could be a bug for including the prefix..
	did, n_actions = os.path.basename(ds).split('.')[0].split('_')[1:]
	did, n_actions = int(did), int(n_actions)
	return n_actions

def get_majority_class(dataset_name):
	maj_class_str = subprocess.check_output(('zcat '+ dataset_name +' | cut -d \' \' -f 1 | sort | uniq -c | sort -r -n | head -1 | xargs '), shell=True)
	maj_size, maj_class = maj_class_str.split()
	return int(maj_size), int(maj_class)

def avg_error(mod):
	return vw_output_extract(mod, 'average loss')

def actual_var(mod):
	return vw_output_extract(mod, 'Measured average variance')

def ideal_var(mod):
	return vw_output_extract(mod, 'Ideal average variance')

def vw_output_extract(mod, pattern):
	#print mod.vw_output_filename
	vw_output = open(mod.vw_output_filename, 'r')
	vw_output_text = vw_output.read()
	#print vw_output_text
	#rgx_pattern = '^'+pattern+' = (.*)(|\sh)\n.*$'
	#print rgx_pattern
	rgx_pattern = '.*'+pattern+' = ([\d]*.[\d]*)( h|)\n.*'
	rgx = re.compile(rgx_pattern, flags=re.M)

	errs = rgx.findall(vw_output_text)
	if not errs:
		avge = 0
	else:
		print errs
		avge = float(errs[0][0])

	vw_output.close()
	return avge

def write_summary_header(mod):
	summary_file = open(mod.summary_file_name, 'w')
	summary_header = disperse(mod.result_header_list, '\t')
	summary_file.write(summary_header+'\n')
	summary_file.close()

def main_loop(mod):
	mod.summary_file_name = mod.results_path+str(mod.task_id)+'of'+str(mod.num_tasks)+'.sum'
	mod.result_template_list = [
	'fold', 0,
	'data', 'ds',
	'num_classes', 0,
	'total_size' , 0,
	'majority_size', 0,
	'corrupt_type_supervised', 0,
	'corrupt_prob_supervised', 0.0,
	'corrupt_type_bandit', 0,
	'corrupt_prob_bandit', 0.0,
	'warm_start', 0,
	'bandit_size', 0,
	'bandit_supervised_size_ratio', 0,
	'cb_type', 'mtr',
	'validation_method', 0,
	'weighting_scheme', 0,
	'lambda_scheme', 0,
	'choices_lambda', 0,
	'no_supervised', False,
	'no_bandit', False,
	'warm_start_type', 0,
	'learning_rate', 0.0,
	'optimal_approx', False,
	'majority_approx', False,
	'avg_error', 0.0,
	'actual_variance', 0.0,
	'ideal_variance', 0.0 ]

 	num_cols = len(mod.result_template_list)/2
	mod.result_header_list = [ mod.result_template_list[2*i] for i in range(num_cols) ]
	mod.result_template = dict([ (mod.result_template_list[2*i], mod.result_template_list[2*i+1]) for i in range(num_cols) ])

	write_summary_header(mod)
	for mod.param in mod.config_task:
		gen_comparison_graph(mod)

def create_dir(dir):
	if not os.path.exists(dir):
		os.makedirs(dir)
		import stat
		os.chmod(dir, os.stat(dir).st_mode | stat.S_IWOTH)

def remove_suffix(filename):
	return os.path.basename(filename).split('.')[0]

if __name__ == '__main__':
	parser = argparse.ArgumentParser(description='vw job')
	parser.add_argument('task_id', type=int, help='task ID, between 0 and num_tasks - 1')
	parser.add_argument('num_tasks', type=int)
	parser.add_argument('--results_dir', default='../../../figs/')
	parser.add_argument('--ds_dir', default='../../../vwshuffled/')
	parser.add_argument('--num_learning_rates', type=int, default=1)
	parser.add_argument('--num_datasets', type=int, default=-1)


	args = parser.parse_args()
	flag_dir = args.results_dir + 'flag/'

	if args.task_id == 0:
		# To avoid race condition of writing to the same file at the same time
		create_dir(args.results_dir)

		# This is specifically designed for teamscratch, as accessing a folder
		# with a huge number of files can be super slow. Hence, we create a subfolder
		# for each dataset to alleviate this.
		dss = ds_files(args.ds_dir + '1/')
		for ds in dss:
			ds_no_suffix = remove_suffix(ds)
			create_dir(args.results_dir + ds_no_suffix + '/')

		create_dir(flag_dir)
	else:
		# may still have the potential of race condition on those subfolders (if
		# we have a lot of datasets to run and the datasets are small)
		while not os.path.exists(flag_dir):
			time.sleep(1)

	mod = model()
	mod.baselines_on = True
	mod.algs_on = True
	mod.optimal_on = False
	mod.majority_on = False

	mod.num_tasks = args.num_tasks
	mod.task_id = args.task_id

	mod.vw_path = '../vowpalwabbit/vw'
	mod.ds_path = args.ds_dir
	mod.results_path = args.results_dir

	mod.num_checkpoints = 200

	# use fractions instead of absolute numbers
	#mod.warm_start_multipliers = [pow(2,i) for i in range(4)]
	mod.warm_start_multipliers = [pow(2,i) for i in range(4)]

	mod.choices_cb_type = ['mtr']
	#mod.choices_choices_lambda = [2,4,8]
	mod.choices_choices_lambda = [2, 4, 8]

	#mod.choices_corrupt_type_supervised = [1,2,3]
	#mod.choices_corrupt_prob_supervised = [0.0,0.5,1.0]
	mod.choices_corrupt_type_supervised = [1,2,3]
	mod.choices_corrupt_prob_supervised = [0.0,0.5,1.0]

	if args.num_learning_rates == 1:
		mod.learning_rates = [0.5]
	elif args.num_learning_rates == 3:
		mod.learning_rates = [0.1, 0.3, 1.0]
	else:
		mod.learning_rates = [0.001, 0.003, 0.01, 0.03, 0.1, 0.3, 1.0, 3.0, 10.0]

	mod.adf_on = True

	mod.corrupt_type_bandit = 1
	mod.corrupt_prob_bandit = 0.0

	mod.validation_method = 1
	mod.weighting_scheme = 1

	mod.epsilon = 0.05
	mod.epsilon_on = True

	mod.critical_size_ratios = [184 * pow(2, -i) for i in range(7) ]

	#mod.folds = range(1,11)
	mod.folds = range(1,6)

	print 'reading dataset files..'
	#TODO: this line specifically for multiple folds
	#Need a systematic way to detect subfolder names
	mod.dss = ds_files(mod.ds_path + '1/')
	print len(mod.dss)

	if args.num_datasets == -1 or args.num_datasets > len(mod.dss):
		pass
	else:
		mod.dss = mod.dss[:args.num_datasets]
	#mod.dss = ["ds_223_63.vw.gz"]
	#mod.dss = mod.dss[:5]

	print 'generating tasks..'
	# here, we are generating the task specific parameter settings
	# by first generate all parameter setting and pick every num_tasks of them
	mod.config_task = params_per_task(mod)
	print 'task ' + str(mod.task_id) + ' of ' + str(mod.num_tasks) + ':'
	print len(mod.config_task)

	#print mod.ds_task

	# we only need to vary the warm start fraction, and there is no need to vary the bandit fraction,
	# as each run of vw automatically accumulates the bandit dataset
	main_loop(mod)
