import matplotlib
import matplotlib.pyplot as plt
import subprocess
import pylab
from itertools import product

class model:
	def __init__(self):
		self.no_bandit = False
		self.no_supervised = False

def collect_stats(mod):

	filename = mod.filename
	# using progress parameter
	# num_rows = mod.bandit / mod.progress



	avg_loss = []
	last_loss = []
	wt = []
	end_table = False

	f = open(filename, 'r')
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

	cmd_catfile = '( head -n ' + str(mod.warm_start) + ' ' + mod.dataset_supervised + ';' + ' head -n ' + str(mod.bandit) + ' ' + mod.dataset_bandit + '; )'
	cmd_vw = mod.vw_path + ' --cbify ' + str(mod.num_classes) + ' --cb_type ' + str(mod.cb_type) + ' --warm_start ' + str(mod.warm_start) + ' --bandit ' + str(mod.bandit) + ' --choices_lambda ' + str(mod.choices_lambda) + alg_option
	#+ ' --progress ' + str(mod.progress)

	cmd = cmd_catfile + ' | ' + cmd_vw

	print cmd

	f = open(mod.filename, 'w')
	process = subprocess.Popen(cmd, shell=True, stdout=f, stderr=f)
	#subprocess.check_call(cmd, shell=True)
	process.wait()
	f.close()

def gen_comparison_graph(mod):

	for mod.warm_start in mod.choices_warm_start:

		config_name = str(mod.fprob1)+'_'+str(mod.fprob2)+'_'+str(mod.warm_start)+'_'+str(mod.bandit)+ '_' + str(mod.cb_type) + '_' + str(mod.choices_lambda)

		# combined approach
		mod.no_bandit = False
		mod.no_supervised = False
		mod.no_exploration = False
		mod.filename = config_name
		execute_vw(mod)
		avg_loss_comb, last_loss_comb, wt_comb = collect_stats(mod)
		line = plt.plot(wt_comb, avg_loss_comb, 'r', label=('Combined approach, #lambdas=' + str(mod.choices_lambda) ))

		# bandit only approach
		mod.no_bandit = False
		mod.no_supervised = True
		mod.no_exploration = False
		mod.filename = config_name+'_no_supervised'
		execute_vw(mod)
		avg_loss_band_only, last_loss_band_only, wt_band_only = collect_stats(mod)
		line = plt.plot(wt_band_only, avg_loss_band_only, 'b', label='Bandit only')

		# supervised only approach
		mod.no_bandit = True
		mod.no_supervised = False
		mod.no_exploration = False
		mod.filename = config_name+'_no_bandit'
		execute_vw(mod)
		avg_loss_sup_only, last_loss_sup_only, wt_sup_only = collect_stats(mod)
		line = plt.plot(wt_sup_only, avg_loss_sup_only, 'g', label='Supervised only')

		pylab.legend()
		pylab.xlabel('#bandit examples')
		pylab.ylabel('Progressive validation error')
		pylab.title('Source 1 feature flipping prob = ' + str(mod.fprob1) + '; source 2 feature flipping prob = ' + str(mod.fprob2) + 'cb_type = '+ mod.cb_type )
		pylab.savefig('figs/'+config_name +'.png')
		plt.gcf().clear()
		print('')
		#plt.show()




if __name__ == '__main__':

	mod = model()

	mod.vw_path = './vowpalwabbit/vw'
	#mod.warm_start = 50
	mod.bandit = 4096
	mod.num_classes = 10
	#mod.cb_type = 'mtr'  #'ips'
    #mod.choices_lambda = 10
	#mod.progress = 25
	mod.adf_on = True

	mod.choices_warm_start = [pow(2,i) for i in range(11)] #put it here in order to plot 2d mesh
	# we are implicitly iterating over the bandit sample sizes
	#choices_fprob1 = [0.1, 0.2, 0.3]
	#choices_fprob2 = [0.1, 0.2, 0.3]
	#choices_cb_types = ['mtr', 'ips']
	choices_choices_lambda = [pow(2,i) for i in range(5)]

	#for correctness test
	#mod.choices_warm_start = [20]
	choices_fprob1 = [0.1]
	choices_fprob2 = [0.1]


	for mod.fprob1, mod.fprob2, mod.cb_type, mod.choices_lambda in product(choices_fprob1, choices_fprob2, choices_cb_types, choices_choices_lambda):
		mod.dataset_supervised = './source1_' + str(mod.fprob1) + '_m.vw'
		mod.dataset_bandit = './source2_' + str(mod.fprob2) + '_m.vw'
		gen_comparison_graph(mod)
