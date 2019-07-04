__author__ = 'aleksei'

from sklearn import metrics

import pandas as pd
import datetime
from dateutil.relativedelta import *

import logging
import os
#import subprocess
import sys
import glob
import re
import argparse

from pyspark import SparkContext, SparkConf, sql

# /user/hdfs/chronicle/parquet/2019*/*/*.parquet
class VWOfflineEvaluation(object):
	def __init__(self, model_name, start, end=None, n_iter=None, verbose=False, test_only=False, downsample=False, add_weights=False, report_baseline=False, **params):
		# init default params
		if not 'train_size' in params or params['train_size'] < 0:
			self.train_size = 1  # train lengh, days
		else:
			self.train_size = params['train_size']
		
		if not 'data_path_train' in params:
			data_path_train = "/user/hdfs/production/output/dcvr_vw_file_"  # path to train data on hdfs
			params['data_path_train'] = data_path_train
		self.data_path_train = params['data_path_train']
		
		if not 'mask_train' in params:
			mask_train = ['*/part-*']
			params['mask_train'] = mask_train
		self.mask_train = params['mask_train']
		
		if not 'pattern_train' in params:
			pattern_train = '{data_path_train}{date}{mask_train}'
			params['pattern_train'] = pattern_train
		self.pattern_train = params['pattern_train']
		
		if not 'vw_train_opts' in params:
			params['vw_train_opts'] = "--kill_cache --loss_function logistic --passes 2 --ftrl --l1 1.0 --l2 0.1 --ftrl_alpha 0.1 --ftrl_beta 1.0 --keep j --keep l --keep q --keep u --keep w --keep A"

		if not 'test_size' in params or params['test_size'] < 0:
			self.test_size = 1  # test length, days
		else:
			self.test_size = params['test_size']
		self.test_size = 1  # ignore passed value, always use 1 to correctly calculate aggregated metrics (AUC, logloss, normalized logloss, calibration etc)

		if not 'data_path_test' in params:
			data_path_test = "/user/hdfs/production/output/dcvr_vw_file_"  # path to test data on hdfs
			params['data_path_test'] = data_path_test
		self.data_path_test = params['data_path_test']
		
		if not 'mask_test' in params:
			mask_test = ['*/part-*']
			params['mask_test'] = mask_test
		self.mask_test = params['mask_test']

		if not 'pattern_test' in params:
			pattern_test = '{data_path_test}{date}{mask_test}'
			params['pattern_test'] = pattern_test
		self.pattern_test = params['pattern_test']

		if not 'output_path' in params:
			params['output_path'] = "/home/datasce/output/"  # path to save model, predictions, cache, logs etc
		
		if not 'vw_test_opts' in params:
			params['vw_test_opts'] = "--loss_function=logistic --link logistic"

		#self.data_path = data_path  
		#self.mask = mask  # file mask(s)
		#self.pattern = pattern  # full pattern of files on hdfs

		#output_path = output_path  # path to save model, predictions, cache, logs etc
		self.model_name = model_name

		self.output_full_path = params['output_path'] + self.model_name if not test_only else self.model_name
		self.model_full_path = self.output_full_path + '.model' if not test_only else self.model_name
		self.cache_full_path = self.output_full_path + '.cache'

		self.pred = pd.DataFrame()  # predictions on test set

		self.__get_logger(params['output_path'], self.model_name)  # init logger

		self.shuffle = False
		self.verbose = verbose
		self.downsample = downsample
		self.add_weights = False if self.downsample else add_weights
		self.report_baseline = report_baseline
		vw_verbosity = ' --quiet' if not self.verbose else ''

		# clean up the output path
		#try:
		#	fileList = glob.glob(self.pred_full_path + '*')
		#	_ = [os.remove(f) for f in fileList]
		#except Exception as e:
		#	pass

		# vowpal train/test options
		#self.vw_train_opts="--loss_function logistic --cache_file {0} --passes 2 --ftrl --l1 1.0 --l2 0.1 --ftrl_alpha 0.1 --ftrl_beta 1.0 --keep j --keep l --keep q --keep u --keep w --keep A --final_regressor {1}".format(self.cache_full_path, self.model_full_path)
		self.vw_train_opts = params['vw_train_opts'] + " --cache_file {0} --final_regressor {1} {2}".format(self.cache_full_path, self.model_full_path, vw_verbosity)
		self.vw_test_opts = params['vw_test_opts'] + vw_verbosity
		self.test_only = test_only

		now = datetime.datetime.now().date()
		start = datetime.datetime.strptime(start, "%Y%m%d").date()  # start(min) date to be used for training, e.g., '2019-05-06'
		end = datetime.datetime.strptime(end, "%Y%m%d").date() if not end is None else (now - pd.DateOffset(1)).strftime("%Y%m%d")  # end(max) date to be used for testing. E.g., '2019-06-06'. If not passed, last available day is used
		#self.train_size = train_size if train_size > 0 else 1  # train lengh, days
		#self.test_size = test_size if test_size > 0 else 1  # test length, days
		#step = 1  # stride
		self.n_iter = n_iter if n_iter > 0 else None # limit number of training iterations (window slides) manually

		# days in interval between start and end
		self.all_days = pd.date_range(pd.to_datetime(start), pd.to_datetime(end))
		
		# N of days between start and end
		total_length = len(self.all_days) - 1
		train_test_length = self.train_size + self.test_size

		if total_length < train_test_length:
			self.logger.error("Interval of days between start and end < train+test length: %d < %d" % (total_length, train_test_length))
			sys.exit(0)
		
		# N of train iterations (number of slides within give interval)
		self.num_iterations = total_length - train_test_length + 1

		# self.session = None  # spark session

		# Verify consistency of passed params
		if not self.n_iter is None and self.n_iter < self.num_iterations:
			self.num_iterations = self.n_iter

		# check whether the start date is available in the hdfs
		# min date in hdfs
		min_hdfs_day = now - relativedelta(months=1) - datetime.timedelta(days=1)

		if (start - min_hdfs_day).days < 0:
			self.logger.error("Start date is too old. Last available day should be at least: %r" % min_hdfs_day)
			raise AssertionError  # ("Start date is too old. Last available day should be at least: %r" % min_hdfs_day)

		self.logger.info('Total days available: %r, train_test_length: %r, num_iterations: %r' % (total_length, train_test_length, self.num_iterations))

	def __get_logger(self, log_path, model_name):
		log_file = "%s.log" % self.output_full_path
		print("Log file: %s" % log_file)

		self.logger = logging.getLogger('ML Offline Evaluation')
		hdlr = logging.FileHandler(filename=log_file)
		formatter = logging.Formatter('%(asctime)s - %(levelname)s - %(message)s')
		hdlr.setFormatter(formatter)
		self.logger.handlers = []
		if len(self.logger.handlers) == 0:
			self.logger.addHandler(hdlr)

		self.logger.setLevel(logging.DEBUG)

		self.logger.info("Log Started")
		#self.self.logger.debug('Using parameters: \n%r' % args)

	def __get_files(self, train_range, test_range):
		train_days = pd.date_range(train_range[0], train_range[1])
		test_days = pd.date_range(test_range[0], test_range[1])
		# print('train_days: %r, test_days: %r' % (train_days, test_days))
		
		# mask for the generated vw strings
		# /user/hdfs/production/output/dcvr_vw_file_2019052405/part-00000
		#mask = ['*/part-*']
		#p = '{data_path}{date}{mask}'

		# mask for the raw data (can be used once spark can overcome an error with blacklisting, see below)
		# data_path + date + '*/joinedData_' + date + '*IMPRESSIONS_NO_CLICKS/*.parquet'
		# ["/user/hdfs/chronicle/parquet/%s*/joinedData_%s*_IMPRESSIONS_NO_CLICKS/*.parquet" % (datehour, datehour),
		#  "/user/hdfs/chronicle/parquet/%s*/joinedData_%s*_INSTALLS/*.parquet" % (datehour, datehour)]
		#
		# mask = ['*IMPRESSIONS_NO_CLICKS/*.parquet', '*INSTALLS/*.parquet']  # for raw parquet data
		# p = '{data_path}{date}*/{mask}'
		
		train_files_list = [self.pattern_train.format(data_path_train=self.data_path_train, date=i.strftime('%Y%m%d'), mask_train=m) for i in train_days for m in self.mask_train]
		test_files_list = [self.pattern_test.format(data_path_test=self.data_path_test, date=i.strftime('%Y%m%d'), mask_test=m) for i in test_days for m in self.mask_test]
		
		# print('train_files_list: %r, test_files_list: %r' % (train_files_list, test_files_list))
		
		# return (train_filenames, test_filenames, train_files_list, test_files_list)
		return (train_files_list, test_files_list)

	def run(self):
		"""
		Train and test model iteratively by sliding a training set of size train_size with the stride of 1 day.
		If --downsample option is passed - also balance the train data on each iteration.
		"""
		for i in range(self.num_iterations):
			pred_file = self.output_full_path + '_%s.pred'%i  # predictions for i-th iteration

			train_start = self.all_days[i]
			train_end = train_start + pd.DateOffset(self.train_size-1)
			
			if not self.test_only:
				test_start = train_end + pd.DateOffset(1)  # days after the train set
			else:
				test_start = train_start
			test_end = test_start + pd.DateOffset(self.test_size - 1)  # start with the very beginning of the interval since there is no train

			self.logger.info('i: %d/%d, train: %r, test: %r' % (i+1, self.num_iterations, (train_start.strftime('%Y%m%d'), train_end.strftime('%Y%m%d')), (test_start.strftime('%Y%m%d'), test_end.strftime('%Y%m%d'))))
			
			train_files_list, test_files_list = self.__get_files((train_start, train_end), (test_start, test_end))
			self.logger.debug('Data sets:\ntrain_files_list: %r, \ntest_files_list: %r' % (train_files_list, test_files_list))
			
			if not self.test_only:
				self.train_iter(train_files_list)
			
			self.test_iter(test_files_list, pred_file)

		self.logger.debug("===========Summary==========")
		self.logger.debug('\nNumber of iterations: %d, \nModel: %s, \nTrain/Test size: %r, \nNormalized predictions: %s' % (self.num_iterations, self.model_full_path, (self.train_size, self.test_size), self.output_full_path))

	def train_iter(self, train_files_list):
		train_filenames = " ".join(train_files_list)

		if self.downsample:
			# train on balanced data
			train_data = self.balance_data(self.output_full_path, train_filenames)
			cmd_train = "cat {0} | vw {1}".format(train_data, self.vw_train_opts)

			# test on balanced data
			#cmd_test = "vw -d {0} -t {1} -i {2} -p {3}".format(test_data, self.vw_test_opts, self.model_full_path, pred_file)
		else:
			# train on raw (unbalanced data)
			if self.add_weights:
				# estimate weights for classes
				#n = os.popen("hadoop fs -cat {0} | grep campaign_type_3 | wc -l".format(train_filenames)).read()  # train lenght
				n = os.popen("hadoop fs -cat {0} | wc -l".format(train_filenames)).read()  # train lenght
				self.logger.debug('n: %r' % n)

				#n_pos = os.popen("hadoop fs -cat {0} | grep campaign_type_3 | grep '^1' | wc -l".format(train_filenames)).read()  # train positive events
				n_pos = os.popen("hadoop fs -cat {0} | grep '^1' | wc -l".format(train_filenames)).read()  # train positive events
				self.logger.debug('n_pos: %r' % n_pos)
				
				n, n_pos = float(n), float(n_pos)

				neg_weight = round(n_pos/n, 5)
				self.logger.debug('neg_weight: %r' % neg_weight)
				self.vw_train_opts += ' --classweight -1:%r' % neg_weight
				self.logger.debug('self.vw_train_opts: %r' % self.vw_train_opts)

			#data_train = spark.read.load(train_files_list).toPandas()
			# Spark fails due to blacklisting error:
			# py4j.protocol.Py4JJavaError: An error occurred while calling o58.collectToPython.: org.apache.spark.SparkException: Job aborted due to stage failure: Aborting TaskSet 2.0 because task 1 (partition 1) cannot run anywhere due to node and executor blacklist.  Blacklisting behavior can be configured via spark.blacklist.*.

			#data_train.head()
			#X_train = data_train.loc[:,features]

			#self.model = VWClassifier()
			#self.model.fit(X_train, y_train)

			shuf_param = 'shuf |' if self.shuffle else ''  # shufle train data or not
			#cmd_train = "hadoop fs -cat {0} | grep campaign_type_3 | vw {1}".format(train_filenames, self.vw_train_opts)
			cmd_train = "hadoop fs -cat {0} | {1} vw {2}".format(train_filenames, shuf_param, self.vw_train_opts)

			self.logger.debug('Training...')
			self.logger.debug(cmd_train)
			os.system(cmd_train)


	def test_iter(self, test_files_list, pred_file):
		test_filenames = " ".join(test_files_list)

		if not os.path.isfile(self.model_full_path):
			self.logger.error("Model file isn't passed")
			return False
		try:
			#cmd_test = "hadoop fs -cat {0} | grep campaign_type_3 | vw {1} -t -i {2} -p {3}".format(test_filenames, self.vw_test_opts, self.model_full_path, pred_file)
			cmd_test = "hadoop fs -cat {0} | vw {1} -t -i {2} -p {3}".format(test_filenames, self.vw_test_opts, self.model_full_path, pred_file)

			self.logger.debug('Testing...')
			self.logger.debug(cmd_test)
			os.system(cmd_test)

			# report ROC-AUC for the i-th iteration
			_ = self.get_metrics(pred_files=[pred_file], logger=self.logger, report_baseline=self.report_baseline)
		except:
			raise

		return True

	@staticmethod
	def get_metrics(pred_files=[], sep=' ', pos_label='PCLASS', logger=None, **params):
		"""
		params:
		pred_files: list of files with ground-true labels and predictions (proba)
		params: some optional args
		"""

		metrics_i = {}  # metrics for i-th file
		all_metrics = []  # all metrics, of type list of dicts

		if len(pred_files) == 0:
			return all_metrics

		# pred_files = sorted(pred_files, key=lambda x:int(re.findall("(\d+)",x)[-1]))  # sort by i
		for f in pred_files:
			# load preds for i-th iteration
			preds = pd.read_csv(filepath_or_buffer=f, sep=sep, names=['pred_prob','true_label'])
			gt = preds.true_label.to_frame()
			p = preds.pred_prob  # .to_frame()

			if not 'report_baseline' in params:
				params['report_baseline'] = False

			number_of_samples = preds.shape[0]
			positive_samples = preds[preds.true_label == 'PCLASS'].shape[0]
			
			avg = positive_samples / float(number_of_samples)
			baseline_score_array = [avg] * number_of_samples

			# PR-AUC
			#precision, recall, pr_threshold = metrics.precision_recall_curve(y_true=gt, probas_pred=p, pos_label=pos_label)
			#pr_auc = metrics.auc(precision, recall)
			pr_avg = metrics.average_precision_score(y_true=gt, y_score=p, pos_label=pos_label)
			# print('pr_avg:%r'%pr_avg)
			# print('pr auc:%r'%pr_auc)

			# ROC-AUC
			fpr, tpr, threshold = metrics.roc_curve(gt, p, pos_label)
			roc_auc = metrics.auc(fpr, tpr)
			# rocauc_score = metrics.roc_auc_score(y_true=gt, y_score=p)
			
			# LogLoss
			logloss = metrics.log_loss(y_true=gt, y_pred=p, normalize=True)
			log_loss_baseline = metrics.log_loss(y_true=gt, y_pred=baseline_score_array, normalize=True)
			log_loss_ratio = logloss / log_loss_baseline

			# Brier score
			brier_score = metrics.brier_score_loss(y_true=gt, y_prob=p, pos_label=pos_label)

			# calibration
			# Shows how far is our predicted average CTR from actual CTR:
			# Avg CTR / Actual CTR = sum(dcvr)/N  / installs/N = sum(dcvr) / installs
			calibration = p.sum() / gt[gt.true_label=='PCLASS'].shape[0]

			metrics_i.update({
				'roc_auc': roc_auc,
				'pr_avg': pr_avg,
				'logloss': logloss, 
				'log_loss_baseline': log_loss_baseline,
				'log_loss_ratio': log_loss_ratio,
				'brier_score': brier_score,
				'calibration': calibration
				})

			# baseline metrics (experimental, handled by '--report_baseline' argument)
			if params['report_baseline']:
				p_baseline = len(p)*[0]  # baseline predictions, assuming we always predict 0
				fpr_b, tpr_b, threshold_b = metrics.roc_curve(gt, p_baseline, pos_label)
				roc_auc_b = metrics.auc(fpr_b, tpr_b)
				metrics_i.update({'roc_auc_b': roc_auc_b})

				logloss_b = metrics.log_loss(y_true=gt, y_pred=p_baseline)  # baseline logloss
			
			if not logger is None:
				if params['report_baseline']:
					logger.info('ROC-AUC: %r, log_loss: %r, ROC-AUC (baseline): %r, log_loss (baseline): %r'  % (round(roc_auc, 4), round(logloss, 4), round(roc_auc_b, 4), round(logloss_b, 4)))
				else:
					logger.info('pos/all: %d/%d, roc-auc: %r, pr_avg: %r, brier: %r, logloss: %r, logloss_baseline: %r, logloss / baseline: %r, calibr: %r'  % 
						(positive_samples, number_of_samples, round(roc_auc, 4), round(pr_avg,4), round(brier_score, 4), round(logloss, 4), round(log_loss_baseline, 4), round(log_loss_ratio, 4), round(calibration, 4)))
			else:
				if params['report_baseline']:
					print('ROC-AUC: %r, log_loss: %r, ROC-AUC (baseline): %r, log_loss (baseline): %r'  % (round(roc_auc, 4), round(logloss, 4), round(roc_auc_b, 4), round(logloss_b, 4)))
				else:
					print('pos/all samples: %d/%d, roc-auc: %r, pr_avg: %r, brier: %r, logloss: %r, logloss_baseline: %r, logloss / baseline: %r, calibr: %r'  % 
						(positive_samples, number_of_samples, round(roc_auc, 4), round(pr_avg,4), round(brier_score, 4), round(logloss, 4), round(log_loss_baseline, 4), round(log_loss_ratio, 4), round(calibration, 4)))

			# appens list with all iterations with i-th metrics
			all_metrics.append(metrics_i)

		return all_metrics

	def __session_get_create(self):
		self.session = SparkSession.builder \
		.appName('OfflineEval')\
		.master('yarn')\
		.config('spark.executor.memory', '8g')\
		.config('spark.executor.instances', 20)\
		.config('spark.executor.cores', 8)\
		.config('spark.network.timeout', 800)\
		.config('spark.sql.execution.arrow.enabled', True)\
		.getOrCreate()


	@staticmethod
	def balance_data(output_full_path='./', train_filenames=None, balance_factor=5, shuffle=True):
		"""
		Balance train data by downsamplig the negative class
		"""
		#shuf = '| shuf ' if shuffle else '>'
		pos_file = '%s_pos.vw' % output_full_path
		neg_file = '%s_neg.vw' % output_full_path
		balanced_file = '%s_all.vw' % output_full_path
		
		# Get positive events
		print("Shufling positive events..")
		shuf_param = '| shuf -o {0}'.format(pos_file) if shuffle else '> {0}'.format(pos_file)
		print('shuffle: %r' % shuffle)
		print('shuf_param: %r' % shuf_param)
		#cmd_pos = "hadoop fs -cat {0} | grep campaign_type_3 | grep '^1' {1}".format(train_filenames, shuf_param)
		cmd_pos = "hadoop fs -cat {0} | grep '^1' {1}".format(train_filenames, shuf_param)
		print('cmd_pos: %r' % cmd_pos)
		os.system(cmd_pos)
		
		# number of positive events
		pos_length = sum(1 for line in open(pos_file))
		print("Number of positive events: %d" % pos_length)

		# Get negative events
		print("Shufling negative events")
		neg_num = int(balance_factor) * pos_length  #  number of negative events to keep
		#print("neg_file: %r" % neg_file)
		shuf_param = '| shuf -n {0} -o {1}'.format(neg_num, neg_file) if shuffle else '| head -n {0} > {1}'.format(neg_num, neg_file)
		#cmd_neg = "hadoop fs -cat {0} | grep campaign_type_3 | grep '^-1' {1}".format(train_filenames, shuf_param)
		cmd_neg = "hadoop fs -cat {0} | grep '^-1' {1}".format(train_filenames, shuf_param)
		print('cmd_neg: %r' % cmd_neg)
		os.system(cmd_neg)
		neg_length = sum(1 for line in open(neg_file))
		print("Number of negative events: %d" % neg_length)

		print("Merging positive and negative events")

		# merge pos and neg events
		shuf_param = '| shuf -o {0}'.format(balanced_file) if shuffle else '> {0}'.format(balanced_file)
		cmd_merge = "paste -d '\n' {0} {1} | sed '/^$/d' {2}".format(pos_file, neg_file, shuf_param)
		print('cmd_merge: %r' % cmd_merge)
		os.system(cmd_merge)

		return balanced_file


	def __tmp_vw_train(self):
		# explore scikit-learn api
		pass

	@staticmethod
	def split_train_test(file_train_test=None, export_path=None):
		"""
		Non-stratified split.
		Example: VWOfflineEvaluation.split_train_test(file_train_test='../../output/tmp_20190618_all.vw', export_path='../../output/')
		"""
		# Split into train/test 90%/10%
		print("Splitting data into train/test (90/10)")
		train_test = '%s_train_test' % file_train_test  # temp file names for train and test data
		train_data = '%s.train'%file_train_test
		test_data = '%s.test'%file_train_test
		cmd_split0 = "split -a 2 -d -l $[ $(wc -l {all_file} | cut -d' ' -f1) * 90 / 100 ] {all_file} {train_test}".format(all_file=file_train_test, train_test=train_test)
		
		# rename files
		cmd_split1 = "mv {train} {train_data} | mv {test} {test_data}".format(
			train='%s00'%train_test, 
			train_data=train_data,
			test='%s01'%train_test, 
			test_data=test_data
			)

		os.system(cmd_split0)
		print('Split done.\nRenaming files..')
		os.system(cmd_split1)

		print('Counting line numbers..')
		all_length = sum(1 for line in open(file_train_test))
		train_length = sum(1 for line in open(train_data))
		test_length = sum(1 for line in open(test_data))
		print('All/Train/Test size: %d/%d/%d' % (all_length, train_length, test_length))
		print('Train file: %r' % train_data)
		print('Test file: %r' % test_data)


if __name__ == '__main__':
	# parse passed arguments
	parser = argparse.ArgumentParser()

	parser.add_argument("-model_name", help='name of the model to be trained or the full model path of the flag --test_only passed', type=str, required=True)
	parser.add_argument("-start", help='start(min) date to be used for training, e.g., \'2019-05-04\'', type=str, required=True)
	parser.add_argument("-end", help='batch size per file', type=str)
	parser.add_argument("-data_path_train", help="path to train data on hdfs. Default: '/user/hdfs/production/output/dcvr_vw_file_'", type=str, default="/user/hdfs/production/output/dcvr_vw_file_")
	parser.add_argument("-data_path_test", help="path to test data on hdfs. Default: '/user/hdfs/production/output/dcvr_vw_file_'", type=str, default="/user/hdfs/production/output/dcvr_vw_file_")
	parser.add_argument("-mask_train", help="Grep(s) of file(s) to be used for training. E.g.: '-mask '*IMPRESSIONS_NO_CLICKS/*.parquet' -mask '*INSTALLS/*.parquet''. Default: '-mask '*/part-*' '", action='append')  # , default=['*/part-*'])
	parser.add_argument("-mask_test", help="Grep(s) of file(s) to be used for testing. E.g.: '-mask '*IMPRESSIONS_NO_CLICKS/*.parquet' -mask '*INSTALLS/*.parquet''. Default: '-mask '*/part-*' '", action='append')  # , default=['*/part-*'])
	parser.add_argument("-pattern_train", help="Full pattern of the train data path on hdfs. E.g., '{data_path}{date}*/{mask}'. Default: '{data_path}{date}{mask}'", type=str, default="{data_path}{date}{mask}")
	parser.add_argument("-pattern_test", help="Full pattern of the test data path on hdfs. E.g., '{data_path}{date}*/{mask}'. Default: '{data_path}{date}{mask}'", type=str, default="{data_path}{date}{mask}")
	parser.add_argument("-output_path", help="path to save model, predictions, cache, logs etc. Default: '/home/datasce/output/'", type=str, default="/home/datasce/output/")
	parser.add_argument("-train_size", help='size of the train sample (moving window). Default: 1', type=int, default=1)
	parser.add_argument("-test_size", help='size of the test sample. Default: 1', type=int, default=1)
	parser.add_argument("-n_iter", help='explicitly limit number of iterations', type=int, required=False)
	parser.add_argument("--verbose", help='verbose. Default: false', action='store_true')
	parser.add_argument("--test_only", help='Do not train the new model. Only test the given model (provide path in parameter -model_name. Default: false', action='store_true')
	parser.add_argument("--downsample", help='Downsample the negative class. Default: false', action='store_true')
	parser.add_argument("--add_weights", help='Adjust weights for classes on each iteration. Default: false', action='store_true')
	parser.add_argument("--report_baseline", help='Report baseline metrics (ROC-AUC, LogLoss): when predictions are all zeros. Default: false', action='store_true')
	parser.add_argument("-vw_train_opts", help="vw train options. Default: '--kill_cache --loss_function logistic --passes 2 --ftrl --l1 1.0 --l2 0.1 --ftrl_alpha 0.1 --ftrl_beta 1.0 --keep j --keep l --keep q --keep u --keep w --keep A'", type=str, required=False, default="--kill_cache --loss_function logistic --passes 2 --ftrl --l1 1.0 --l2 0.1 --ftrl_alpha 0.1 --ftrl_beta 1.0 --keep j --keep l --keep q --keep u --keep w --keep A")
	parser.add_argument("-vw_test_opts", help="vw test options. Use --link glf1 or --link logistic to scale probs to [-1;1] and [0;1] correspondingly. Default: '--loss_function logistic --link logistic'", type=str, required=False, default="--loss_function logistic --link logistic")
	
	args = parser.parse_args()

	# sc = SparkContext()
	# spark = sql.SQLContext(sc)
	if args.mask_train is None:
		args.mask_train = ['*/part-*']
	if args.mask_test is None:
		args.mask_test = ['*/part-*']

	oe = VWOfflineEvaluation(
		model_name=args.model_name, 
		start=args.start, 
		end=args.end, 
		data_path_train=args.data_path_train,
		mask_train=args.mask_train,
		pattern_train=args.pattern_train,
		data_path_test=args.data_path_test,
		mask_test=args.mask_test,
		pattern_test=args.pattern_test,
		output_path=args.output_path,
		train_size=args.train_size,
		test_size=args.test_size,
		n_iter=args.n_iter,
		vw_train_opts=args.vw_train_opts,
		vw_test_opts=args.vw_test_opts,
		verbose=args.verbose,
		test_only=args.test_only,
		downsample=args.downsample,
		add_weights=args.add_weights,
		report_baseline=args.report_baseline
		)

	oe.run()

# Test
'''
python oe.py -h
# Case 1: train + test
python oe.py -model_name 'tmp_20190627' -start '20190524' -train_size 2 -test_size 2 -n_iter 3
python oe.py \
-model_name 'dcvr_20190627_2d' \
-data_path_train '/user/hdfs/production/output/dcvr_vw_file_' \
-mask_train '*/part-*' \
-pattern_train '{data_path_train}{date}{mask_train}' \
-data_path_test '/user/datasce/production/output/dcvr9_vw_file_' \
-mask_test '*/part-*' \
-pattern_test '{data_path_test}{date}{mask_test}' \
-start '20190601' \
-train_size 14 \
-test_size 2 \
-vw_train_opts '--kill_cache --loss_function logistic --passes 4 --ftrl --l1 1.0 --l2 0.1 --ftrl_alpha 0.1 --ftrl_beta 1.0 --keep j --keep l --keep q --keep u --keep w --keep A' \
-output_path '/home/datasce/output/dcvr/'

# Case 2: test only
# python oe.py -model_name '/home/datasce/alex_ds/OfflineEvaluation/best_unbalanced.model' -start '20190525' -test_size 2 --test_only
'''

# Test by calling python class
'''
from oe import *
oe = VWOfflineEvaluation(model_name='tmp_20190621', start='20190520', train_size=14, test_size=2, n_iter=1, downsample=False)
oe.run()
'''

# Test with spark (spark blacklisting error should be fixed first)
# spark-submit --num-executors 4 --executor-memory 1g --name 'alex_explore' --conf spark.network.timeout=800 --conf spark.executor.cores=8 --conf spark.sql.execution.arrow.enabled=True oe.py -model_name 'tmp_20190625' -start '20190524' -train_size 2 -test_size 2 -n_iter 3 -data_path /user/hdfs/chronicle/parquet/

# TODO:
# (1) warm start (continue training on next iterations instead of training from scratch)
# (2) add shuffle for unbalanced data



