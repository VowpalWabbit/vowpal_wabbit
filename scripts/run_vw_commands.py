import subprocess
from itertools import product
import os
import math
import argparse
import time
import glob
import re
from collections import OrderedDict


class model:
	def __init__(self):
		# Setting up argument-independent learning parameters in the constructor
		self.baselines_on = True
		self.algs_on = True
		self.optimal_on = True
		self.majority_on = True

		self.num_checkpoints = 200

		# use fractions instead of absolute numbers
		self.ws_multipliers = [pow(2,i) for i in range(4)]

		self.choices_cb_type = ['mtr']
		#mod.choices_choices_lambda = [2,4,8]
		self.choices_choices_lambda = [2,8,16]

		#mod.choices_cor_type_ws = [1,2,3]
		#mod.choices_cor_prob_ws = [0.0,0.5,1.0]
		self.choices_cor_type_ws = [1]
		self.choices_cor_prob_ws = [0.0]

		self.choices_cor_type_inter = [1,2,3]
		self.choices_cor_prob_inter = [0.0,0.5,1.0]

		self.validation_method = 1
		self.weighting_scheme = 2

		#self.epsilon = 0.05
		#self.epsilon_on = True
		self.lr_template = [0.1, 0.03, 0.3, 0.01, 1.0, 0.003, 3.0, 0.001, 10.0]
		self.adf_on = True
		self.critical_size_ratios = [184 * pow(2, -i) for i in range(7) ]


def collect_stats(mod):
	avg_error_value = avg_error(mod)
	actual_var_value = actual_var(mod)
	ideal_var_value = ideal_var(mod)

	vw_run_results = []
	vw_result_template = {
	'interaction': 0,
	'inter_ws_size_ratio': 0,
	'avg_error': 0.0,
	'actual_variance': 0.0,
	'ideal_variance': 0.0
	}

	if 'majority_approx' in mod.param or 'optimal_approx' in mod.param:
		vw_result = vw_result_template.copy()
		if 'optimal_approx' in mod.param:
			# this condition is for computing the optimal error
			vw_result['avg_error'] = avg_error_value
		else:
			# this condition is for computing the majority error
			err =  1 - float(mod.param['majority_size']) / mod.param['total_size']
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
			inter_effective = int(float(weight_str))

			for ratio in mod.critical_size_ratios:
				if inter_effective >= (1 - 1e-7) * mod.param['warm_start'] * ratio and \
				inter_effective <= (1 + 1e-7) * mod.param['warm_start'] * ratio:
					vw_result = vw_result_template.copy()
					vw_result['interaction'] = inter_effective
					vw_result['inter_ws_size_ratio'] = ratio
					vw_result['avg_error'] = avg_loss
					vw_result['actual_variance'] = actual_var_value
					vw_result['ideal_variance'] = ideal_var_value
					vw_run_results.append(vw_result)
	f.close()

	#if len(vw_run_results) >= 1:
	#	print mod.param['warm_start']
	#	print vw_run_results
	#raw_input('..')
	return vw_run_results


def gen_vw_options_list(mod):
	mod.vw_options = format_setting(mod.vw_template, mod.param)
	vw_options_list = []
	for k, v in mod.vw_options.iteritems():
		vw_options_list.append('--'+str(k))
		vw_options_list.append(str(v))
	return vw_options_list

def gen_vw_options(mod):
	if 'optimal_approx' in mod.param:
		# Fully supervised on full dataset
		mod.vw_template = OrderedDict([('data',''),
									   ('progress',2.0),
									   ('passes',0),
									   ('oaa',0),
									   ('cache_file','')])
		mod.param['passes'] = 5
		mod.param['oaa'] = mod.param['num_classes']
		mod.param['cache_file'] = mod.param['data'] + '.cache'
	elif 'majority_approx' in mod.param:
		# Compute majority error; basically we would like to skip vw running as fast as possible
		mod.vw_template = OrderedDict([('data',''),
									   ('progress',2.0),
									   ('cbify',0),
									   ('warm_start',0),
									   ('interaction',0)])
		mod.param['cbify'] = mod.param['num_classes']
		mod.param['warm_start'] = 0
		mod.param['interaction'] = 0
	else:
		# General CB
		mod.vw_template = OrderedDict([('data',''),
									   ('cbify',0),
									   ('cb_type','mtr'),
									   ('warm_start',0),
									   ('interaction',0),
									   ('corrupt_type_interaction',0),
									   ('corrupt_prob_interaction',0.0),
									   ('corrupt_type_warm_start',0),
									   ('corrupt_prob_warm_start',0.0),
									   ('warm_start_update',True),
									   ('interaction_update',True),
									   ('choices_lambda',0),
									   ('lambda_scheme',1),
									   ('warm_start_type',1),
									   ('overwrite_label',1),
									   ('validation_method',1),
									   ('weighting_scheme',1),
									   ('learning_rate',0.5),
   									   ('progress',2.0),])

		mod.param['warm_start'] = mod.param['warm_start_multiplier'] * mod.param['progress']
		mod.param['interaction'] = mod.param['total_size'] - mod.param['warm_start']
		mod.param['cbify'] = mod.param['num_classes']
		mod.param['overwrite_label'] = mod.param['majority_class']

		if mod.param['adf_on'] is True:
			mod.param['cb_explore_adf'] = ' '
			mod.vw_template['cb_explore_adf'] = ' '
		else:
			mod.param['cb_explore'] = mod.param['num_classes']
			mod.vw_template['cb_explore'] = 0


def execute_vw(mod):
	gen_vw_options(mod)
	vw_options_list = gen_vw_options_list(mod)
	cmd = intersperse([mod.vw_path]+vw_options_list, ' ')
	print cmd

	f = open(mod.vw_output_filename, 'w')
	process = subprocess.Popen(cmd, shell=True, stdout=f, stderr=f)
	#subprocess.check_call(cmd, shell=True)
	process.wait()
	f.close()

def intersperse(l, ch):
	s = ''
	for item in l:
		s += str(item)
		s += ch
	return s

def param_to_str(param):
	param_list = [ str(k)+'='+str(v) for k,v in param.iteritems() ]
	return intersperse(param_list, ',')

def replace_keys(dic, simplified_keymap):
	dic_new = OrderedDict()
	for k, v in dic.iteritems():
		dic_new[simplified_keymap[k]] = v
	return dic_new

def param_to_str_simplified(mod):
	#print 'before replace'
	#print param
	vw_run_param_set = \
	['dataset',
	 'fold',
	 'lambda_scheme',
	 'validation_method',
	 'warm_start_multiplier',
	 'corrupt_prob_interaction',
	 'corrupt_prob_warm_start',
	 'corrupt_type_interaction',
	 'corrupt_type_warm_start',
 	 'warm_start_update',
 	 'interaction_update',
	 'warm_start_type',
	 'choices_lambda',
	 'weighting_scheme',
	 'cb_type',
	 'optimal_approx',
	 'majority_approx',
	 'learning_rate',
	 'adf_on']

	mod.template_red = OrderedDict([(k,mod.result_template[k]) for k in vw_run_param_set])
	#mod.simplified_keymap_red = dict([(k,mod.simplified_keymap[k]) for k in vw_run_param_set])
	# step 1: use the above as a template to filter out irrelevant parameters
	# in the vw output file title
	param_formatted = format_setting(mod.template_red, mod.param)
	# step 2: replace the key names with the simplified names
	param_simplified = replace_keys(param_formatted, mod.simplified_keymap)
	#print 'after replace'
	#print param
	return param_to_str(param_simplified)

def run_single_expt(mod):
	mod.param['data'] = mod.ds_path + str(mod.param['fold']) + '/' + mod.param['dataset']
	mod.param['total_size'] = get_num_lines(mod.param['data'])
	mod.param['num_classes'] = get_num_classes(mod.param['data'])
	mod.param['majority_size'], mod.param['majority_class'] = get_majority_class(mod.param['data'])
	mod.param['progress'] = int(math.ceil(float(mod.param['total_size']) / float(mod.num_checkpoints)))
	mod.vw_output_dir = mod.results_path + remove_suffix(mod.param['data']) + '/'
	mod.vw_output_filename = mod.vw_output_dir + param_to_str_simplified(mod) + '.txt'

	#plot_errors(mod)
	#print mod.param['validation_method']

	execute_vw(mod)
	vw_run_results = collect_stats(mod)
	for vw_result in vw_run_results:
		result_combined = merge_two_dicts(mod.param, vw_result)

		#print mod.result_template['no_interaction_update']
		#print result_combined['no_interaction_update']

		result_formatted = format_setting(mod.result_template, result_combined)
		record_result(mod, result_formatted)


# The following function is a "template filling" function
# Given a template, we use the setting dict to fill it as much as possible
def format_setting(template, setting):
	formatted = template.copy()
	for k, v in setting.iteritems():
		if k in template.keys():
			formatted[k] = v
	return formatted

def record_result(mod, result):
	result_row = result.values()
	#for k in mod.result_header_list:
	#	result_row.append(result[k])
	#print result['validation_method']
	#print result_row

	summary_file = open(mod.summary_file_name, 'a')
	summary_file.write( intersperse(result_row, '\t') + '\n')
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
	print param_name, len(result)
	return result

def params_per_task(mod):
	# Problem parameters
	prm_cor_type_ws = dictify('corrupt_type_warm_start', mod.choices_cor_type_ws)
	prm_cor_prob_ws = dictify('corrupt_prob_warm_start', mod.choices_cor_prob_ws)
	prm_cor_type_inter = dictify('corrupt_type_interaction', mod.choices_cor_type_inter)
	prm_cor_prob_inter = dictify('corrupt_prob_interaction', mod.choices_cor_prob_inter)
	prm_ws_multiplier = dictify('warm_start_multiplier', mod.ws_multipliers)
	prm_lrs = dictify('learning_rate', mod.learning_rates)
	# could potentially induce a bug if the maj and best does not have this parameter
	prm_fold = dictify('fold', mod.folds)
	# Algorithm parameters
	prm_cb_type = dictify('cb_type', mod.choices_cb_type)
	prm_dataset = dictify('dataset', mod.dss)
	prm_choices_lbd = dictify('choices_lambda', mod.choices_choices_lambda)
	prm_adf_on = dictify('adf_on', [True])

	# Common parameters
	prm_com = param_cartesian_multi(
	[prm_cor_type_ws,
	 prm_cor_prob_ws,
	 prm_cor_type_inter,
	 prm_cor_prob_inter,
	 prm_ws_multiplier,
	 prm_lrs,
	 prm_cb_type,
	 prm_fold,
	 prm_adf_on])

	fltr_inter_gt = lambda p: ((p['corrupt_type_interaction'] == 1 #noiseless for interaction data
							and abs(p['corrupt_prob_interaction']) < 1e-4)
							and
		                    (p['corrupt_type_warm_start'] == 1 #filter out repetitive warm start data
							or abs(p['corrupt_prob_warm_start']) > 1e-4))

	prm_com_inter_gt = filter(fltr_inter_gt, prm_com)

	fltr_ws_gt = lambda p: ((p['corrupt_type_warm_start'] == 1 #noiseless for warm start data
						and abs(p['corrupt_prob_warm_start']) < 1e-4)
						and
	                    (p['corrupt_type_interaction'] == 1 #filter out repetitive interaction data
						or abs(p['corrupt_prob_interaction']) > 1e-4))

	prm_com_ws_gt = filter(fltr_ws_gt, prm_com)

	prm_com = filter(lambda p: (fltr_ws_gt(p) or fltr_inter_gt(p)), prm_com)

	# Baseline parameters construction
	if mod.baselines_on:
		prm_baseline_basic = \
		[
			[
				#Sup-Only
		 		{'warm_start_type': 1,
				 'warm_start_update': True,
				 'interaction_update': False},
				#Band-Only
 		 		{'warm_start_type': 1,
 				 'warm_start_update': False,
 				 'interaction_update': True},
				#Sim-Bandit
				{'warm_start_type': 2,
				 'warm_start_update': True,
 				 'interaction_update': True},
				#Sim-Bandit with no warm-start update
				{'warm_start_type': 2,
				 'warm_start_update': True,
 				 'interaction_update': False}
			]
		]

		prm_baseline_const = \
		[
			[
				{'weighting_scheme':1,
				 'adf_on':True,
				 'lambda_scheme':3,
				 'choices_lambda':1}
			]
		]
		prm_baseline = param_cartesian_multi([prm_com] + prm_baseline_const + prm_baseline_basic)
	else:
		prm_baseline = []


	# Algorithm parameters construction
	if mod.algs_on:
		# Algorithms for supervised validation
		prm_ws_gt = \
		[
			 [
		  	 	{'warm_start_update': True,
				 'interaction_update': True,
				 'warm_start_type': 1,
				 'lambda_scheme': 2,
				 'weighting_scheme': 2}
			 ],
			 [
			 	{'validation_method':2},
				{'validation_method':3}
			 ]
	    ]

		prm_inter_gt = \
		[
			 [
		  	 	{'warm_start_update': True,
				 'interaction_update': True,
				 'warm_start_type': 1,
				 'lambda_scheme': 4,
				 'weighting_scheme': 1}
			 ],
		]

		prm_algs_ws_gt = param_cartesian_multi([prm_com_ws_gt] + [prm_choices_lbd] + prm_ws_gt)
		prm_algs_inter_gt = param_cartesian_multi([prm_com_inter_gt] + [prm_choices_lbd] + prm_inter_gt)
		prm_algs = prm_algs_ws_gt + prm_algs_inter_gt
	else:
		prm_algs = []

	# Optimal baselines parameter construction
	if mod.optimal_on:
		prm_optimal = \
		[
			{'optimal_approx': True,
			 'fold': 1,
			 'corrupt_type_warm_start':1,
			 'corrupt_prob_warm_start':0.0,
			 'corrupt_type_interaction':1,
			 'corrupt_prob_interaction':0.0}
	    ]
	else:
		prm_optimal = []

	if mod.majority_on:
		prm_majority = \
		[
			{'majority_approx': True,
			 'fold': 1,
			 'corrupt_type_warm_start':1,
			 'corrupt_prob_warm_start':0.0,
			 'corrupt_type_interaction':1,
			 'corrupt_prob_interaction':0.0}
		]
	else:
		prm_majority = []


	#for p in params_common:
	#	print p
	#for p in params_baseline:
	#	print p
	#print len(prm_com_ws_gt), len(prm_algs_ws_gt)
	#print len(prm_com_inter_gt), len(prm_algs_inter_gt)
	#print len(prm_com)
	#print len(prm_baseline)
	#print len(prm_algs)
	#raw_input('..')

	# Common factor in all 3 groups: dataset
	prm_all = param_cartesian_multi(
	[prm_dataset,

	 prm_baseline + prm_algs + prm_optimal + prm_majority])

	prm_all = sorted(prm_all,
						key=lambda d: (d['dataset'],
						               d['corrupt_type_warm_start'],
									   d['corrupt_prob_warm_start'],
									   d['corrupt_type_interaction'],
									   d['corrupt_prob_interaction'])
					   )
	print 'The total number of VW commands to run is: ', len(prm_all)
	#for row in prm_all:
	#	print row
	return get_params_task(prm_all)


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
		#print errs
		avge = float(errs[0][0])

	vw_output.close()
	return avge

def write_summary_header(mod):
	summary_file = open(mod.summary_file_name, 'w')
	summary_header = intersperse(mod.result_template.keys(), '\t')
	summary_file.write(summary_header+'\n')
	summary_file.close()

def main_loop(mod):
	mod.summary_file_name = mod.results_path+str(mod.task_id)+'of'+str(mod.num_tasks)+'.sum'

	# The reason for using a list is that, we would like to keep the order of the
	#columns in this way. Maybe use ordered dictionary in the future?
	mod.result_template_list = [
	('fold', 'fd', 0),
	('data', 'dt', ''),
	('dataset', 'ds', ''),
	('num_classes','nc', 0),
	('total_size', 'ts', 0),
	('majority_size','ms', 0),
	('corrupt_type_warm_start', 'ctws', 0),
	('corrupt_prob_warm_start', 'cpws', 0.0),
	('corrupt_type_interaction', 'cti', 0),
	('corrupt_prob_interaction', 'cpi', 0.0),
	('adf_on', 'ao', True),
	('warm_start_multiplier','wsm',1),
	('warm_start', 'ws', 0),
	('warm_start_type', 'wst', 0),
	('interaction', 'bs', 0),
	('inter_ws_size_ratio', 'iwsr', 0),
	('cb_type', 'cbt', 'mtr'),
	('validation_method', 'vm', 0),
	('weighting_scheme', 'wts', 0),
	('lambda_scheme', 'ls', 0),
	('choices_lambda', 'cl', 0),
	('warm_start_update', 'wsu', True),
	('interaction_update', 'iu', True),
	('learning_rate', 'lr', 0.0),
	('optimal_approx', 'oa', False),
	('majority_approx', 'ma', False),
	('avg_error', 'ae', 0.0),
	('actual_variance', 'av', 0.0),
	('ideal_variance', 'iv', 0.0),
	('last_lambda', 'll', 0.0),
	]

 	num_cols = len(mod.result_template_list)
	#mod.result_header_list = [ mod.result_template_list[i][0] for i in range(num_cols) ]
	mod.result_template = OrderedDict([ (mod.result_template_list[i][0], mod.result_template_list[i][2]) for i in range(num_cols) ])
	mod.simplified_keymap = OrderedDict([ (mod.result_template_list[i][0], mod.result_template_list[i][1]) for i in range(num_cols) ])

	write_summary_header(mod)
	for mod.param in mod.config_task:
		#if (mod.param['no_interaction_update'] is True):
		#	raw_input(' ')
		run_single_expt(mod)

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
	parser.add_argument('--num_folds', type=int, default=1)

	args = parser.parse_args()
	flag_dir = args.results_dir + 'flag/'

	mod = model()
	mod.num_tasks = args.num_tasks
	mod.task_id = args.task_id
	mod.vw_path = '../vowpalwabbit/vw'
	mod.ds_path = args.ds_dir
	mod.results_path = args.results_dir
	print 'reading dataset files..'
	#TODO: this line specifically for multiple folds
	#Need a systematic way to detect subfolder names
	mod.dss = ds_files(mod.ds_path + '1/')

	print len(mod.dss)

	if args.num_datasets == -1 or args.num_datasets > len(mod.dss):
		pass
	else:
		mod.dss = mod.dss[:args.num_datasets]

	#print mod.dss

	if args.task_id == 0:
		# Compile vw in one of the subfolders
		process = subprocess.Popen('make -C .. clean; make -C ..', shell=True, stdout=f, stderr=f)
		subprocess.check_call(cmd, shell=True)
		process.wait()

		# To avoid race condition of writing to the same file at the same time
		create_dir(args.results_dir)

		# This is specifically designed for teamscratch, as accessing a folder
		# with a huge number of result files can be super slow. Hence, we create a
		# subfolder for each dataset to alleviate this.
		for ds in mod.dss:
			ds_no_suffix = remove_suffix(ds)
			create_dir(args.results_dir + ds_no_suffix + '/')

		create_dir(flag_dir)
	else:
		# may still have the potential of race condition on those subfolders (if
		# we have a lot of datasets to run and the datasets are small)
		while not os.path.exists(flag_dir):
			time.sleep(1)

	if args.num_learning_rates <= 0 or args.num_learning_rates >= 10:
		mod.learning_rates = mod.lr_template
	else:
		mod.learning_rates = mod.lr_template[:args.num_learning_rates]
	#mod.folds = range(1,11)
	mod.folds = range(1, args.num_folds+1)

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
