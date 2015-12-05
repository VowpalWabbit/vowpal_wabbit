# coding: utf-8

"""
Github version of hyperparameter optimization for Vowpal Wabbit via hyperopt
"""

__author__ = 'kurtosis'

from hyperopt import hp, fmin, tpe, rand, Trials, STATUS_OK
from sklearn.metrics import roc_curve, auc, log_loss, precision_recall_curve
import numpy as np
from datetime import datetime as dt
import subprocess, shlex
from math import exp, log
import argparse
import re
import logging


def read_arguments():
    parser = argparse.ArgumentParser()

    parser.add_argument('--searcher', type=str, default='tpe', choices=['tpe', 'rand'])
    parser.add_argument('--max_evals', type=int, default=100)
    parser.add_argument('--train', type=str, required=True, help="training set")
    parser.add_argument('--holdout', type=str, required=True, help="holdout set")
    parser.add_argument('--vw_space', type=str, required=True, help="hyperparameter search space (must be 'quoted')")
    #parser.add_argument('--loss_function', default='logistic',
    #                    choices=['logistic', 'squared', 'hinge', 'quantile', 'classic'])
    parser.add_argument('--outer_loss_function', default='logistic', choices=['logistic', 'roc-auc'])  # TODO: implement squared, hinge, quantile, PR-auc
    parser.add_argument('--regression', action='store_true', default=False, help="""regression (continuous class labels)
                                                                        or classification (-1 or 1, default value).""")

    args = parser.parse_args()
    return args


class HyperoptSpaceConstructor(object):
    """
    Takes command-line input and transforms it into hyperopt search space
    """

    def __init__(self, command=("--algorithms=[ftrl,sgd] --l2=[1e-8..1e-4]LO "
                                "-l=[0.01..10]L --ftrl_beta=[0.01..1] "
                                "--passes=[1..10]I -q=[SE+SZ+DR,SE]O")):
        self.command = command
        self.space = None
        self.algorithm_metadata = {
            'ftrl': {'arg': '--ftrl', 'prohibited_flags': set()},
            'sgd': {'arg': '', 'prohibited_flags': {'--ftrl_alpha', '--ftrl_beta'}}
        }

        self.range_pattern = re.compile("(?<=\[).+(?=\])")
        self.distr_pattern = re.compile("(?<=\])[IOL]*")
        self.only_continuous = re.compile("(?<=\])[IL]*")

    def _process_vw_argument(self, arg, value, algorithm):
        distr_part = self.distr_pattern.findall(value)[0]
        range_part = self.range_pattern.findall(value)[0]
        is_continuous = '..' in range_part

        if not is_continuous and self.only_continuous.findall(value)[0]!='':
            raise ValueError(("Need a range instead of a list of discrete values to define "
                              "uniform or log-uniform distribution. "
                              "Please, use [min..max]%s form") % (distr_part))

        if is_continuous and arg == '-q':
            raise ValueError(("You must directly specify namespaces for quadratic features "
                              "as a list of values, not as a parametric distribution"))

        hp_choice_name = "_".join([algorithm, arg.replace('-','')])

        try_omit_zero = 'O' in distr_part
        distr_part = distr_part.replace('O','')

        if is_continuous:
            vmin, vmax = [float(i) for i in range_part.split('..')]

            if distr_part == 'L':
                distrib = hp.loguniform(hp_choice_name, log(vmin), log(vmax))
            elif distr_part == '':
                distrib = hp.uniform(hp_choice_name, vmin, vmax)
            elif distr_part == 'I':
                distrib = hp.quniform(hp_choice_name, vmin, vmax, 1)
            elif distr_part in {'LI', 'IL'}:
                distrib = hp.qloguniform(hp_choice_name, log(vmin), log(vmax), 1)
            else:
                raise ValueError("Cannot recognize distribution: %s" % (distr_part))
        else:
            possible_values = range_part.split(',')
            if arg == '-q':
                possible_values = [v.replace('+',' -q ') for v in possible_values]
            distrib = hp.choice(hp_choice_name, possible_values)

        if try_omit_zero:
            hp_choice_name_outer = hp_choice_name + '_outer'
            distrib = hp.choice(hp_choice_name_outer, ['omit', distrib])

        return distrib

    def string_to_pyll(self):
        line = shlex.split(self.command)

        algorithms = ['sgd']
        for arg in line:
            arg, value = arg.split('=')
            if arg == '--algorithms':
                algorithms = set(self.range_pattern.findall(value)[0].split(','))
                for algo in algorithms:
                    if algo not in self.algorithm_metadata:
                        raise NotImplementedError(("%s algorithm is not found. "
                                                   "Supported algorithms by now are %s")
                                                  % (algo, str(self.algorithm_metadata.keys())))
                break

        self.space = {algo:{'type':algo, 'argument':self.algorithm_metadata[algo]['arg']} for algo in algorithms}
        for algo in algorithms:
            for arg in line:
                arg, value = arg.split('=')
                if arg == '--algorithms':
                    continue
                distrib = self._process_vw_argument(arg, value, algo)
                if arg not in self.algorithm_metadata[algo]['prohibited_flags']:
                    self.space[algo][arg] = distrib
        self.space = hp.choice('algorithm', self.space.values())


class HyperOptimizer(object):
    def __init__(self, train_set, holdout_set, command, max_evals=100,
                 outer_loss_function='logistic',
                 searcher='tpe', is_regression=False):
        self.logger = self._configure_logger()

        self.train_set = train_set
        self.holdout_set = holdout_set

        self.train_model = 'current.model'
        self.holdout_pred = 'holdout.pred'
        self.holdout_metrics = 'holdout_metrics'

        # hyperopt parameter sample, converted into a string with flags
        self.param_suffix = None
        self.train_command = None
        self.validate_command = None

        self.y_true_train = []
        self.y_true_holdout = []

        self.outer_loss_function = outer_loss_function
        self.space = self._get_space(command)
        self.max_evals = max_evals
        self.searcher = searcher
        self.is_regression = is_regression

    def _get_space(self, command):
        hs = HyperoptSpaceConstructor(command)
        hs.string_to_pyll()
        return hs.space

    def _configure_logger(self):
        LOGGER_FORMAT = "%(asctime)s,%(msecs)03d %(levelname)-8s [%(name)s/%(module)s:%(lineno)d]: %(message)s"
        LOGGER_DATEFMT = "%Y-%m-%d %H:%M:%S"
        LOGFILE = "./log.log"

        logging.basicConfig(format=LOGGER_FORMAT,
                            datefmt=LOGGER_DATEFMT,
                            level=logging.DEBUG)
        formatter = logging.Formatter(LOGGER_FORMAT, datefmt=LOGGER_DATEFMT)

        file_handler = logging.FileHandler(LOGFILE)
        file_handler.setFormatter(formatter)

        logger = logging.getLogger()
        logger.addHandler(file_handler)
        return logger

    def get_hyperparam_string(self, **kwargs):
        if '--passes' in kwargs:
            kwargs['--passes'] = int(kwargs['--passes'])

        flags = [key for key in kwargs if key.startswith('-')]
        for flag in flags:
            if kwargs[flag] == 'omit':
                del kwargs[flag]

        self.param_suffix = ' '.join(['%s %s' % (key, kwargs[key]) for key in kwargs if key.startswith('-')])
        self.param_suffix += ' %s' % (kwargs['argument'])

    def compose_vw_train_command(self):
        data_part = ('vw -d %s -f %s --holdout_off -c '
                     % (self.train_set, self.train_model))
        self.train_command = ' '.join([data_part, self.param_suffix])

    def compose_vw_validate_command(self):
        data_part = 'vw -t -d %s -i %s -p %s --holdout_off -c' \
                    % (self.holdout_set, self.train_model, self.holdout_pred)
        self.validate_command = data_part

    def fit_vw(self):
        self.compose_vw_train_command()
        self.logger.info("executing the following command (training): %s" % self.train_command)
        subprocess.call(shlex.split(self.train_command))

    def validate_vw(self):
        self.compose_vw_validate_command()
        self.logger.info("executing the following command (validation): %s" % self.validate_command)
        subprocess.call(shlex.split(self.validate_command))

    def get_y_true_train(self):
        self.logger.info("loading true train class labels...")
        yh = open(self.train_set, 'r')
        self.y_true_train = []
        for line in yh:
            self.y_true_train.append(int(line.strip()[0:2]))
        if not self.is_regression:
            self.y_true_train = [(i + 1.) / 2 for i in self.y_true_train]
        self.logger.info("train length: %d" % len(self.y_true_train))

    def get_y_true_holdout(self):
        self.logger.info("loading true holdout class labels...")
        yh = open(self.holdout_set, 'r')
        self.y_true_holdout = []
        for line in yh:
            self.y_true_holdout.append(int(line.strip()[0:2]))
        if not self.is_regression:
            self.y_true_holdout = [(i + 1.) / 2 for i in self.y_true_holdout]
        self.logger.info("holdout length: %d" % len(self.y_true_holdout))

    def validation_metric_vw(self):
        v = open('%s' % self.holdout_pred, 'r')
        y_pred_holdout = []
        for line in v:
            y_pred_holdout.append(float(line.strip()))

        if self.outer_loss_function == 'logistic':
            y_pred_holdout_proba = [1. / (1 + exp(-i)) for i in y_pred_holdout]
            loss = log_loss(self.y_true_holdout, y_pred_holdout_proba)

        elif self.outer_loss_function == 'squared':  # TODO: write it
            pass

        elif self.outer_loss_function == 'hinge':  # TODO: write it
            pass

        elif self.outer_loss_function == 'roc-auc':
            y_pred_holdout_proba = [1. / (1 + exp(-i)) for i in y_pred_holdout]
            fpr, tpr, _ = roc_curve(self.y_true_holdout, y_pred_holdout_proba)
            loss = auc(fpr, tpr)

        m = open(self.holdout_metrics, 'a+')
        m.write('%s\t%s\n' % (self.param_suffix, loss))
        m.close()

        return loss

    def hyperopt_search(self, parallel=False):  # TODO: implement parallel search with MongoTrials
        def objective(kwargs):
            start = dt.now()
            self.get_hyperparam_string(**kwargs)
            self.fit_vw()
            self.validate_vw()
            loss = self.validation_metric_vw()

            finish = dt.now()
            elapsed = finish - start
            self.logger.info("evaluation time for this step: %s" % str(elapsed))

            # clean up
            subprocess.call(shlex.split('rm %s %s' % (self.train_model, self.holdout_pred)))

            to_return = {'status': STATUS_OK,
                         'loss': loss,  # TODO: include also train loss tracking in order to prevent overfitting
                         'eval_time': elapsed,
                         'train_command': self.train_command
                        }
            return to_return

        trials = Trials()
        if self.searcher == 'tpe':
            algo = tpe.suggest
        elif self.searcher == 'rand':
            algo = rand.suggest

        logging.debug("starting hypersearch...")
        best_params = fmin(objective, space=self.space, trials=trials, algo=algo, max_evals=self.max_evals)
        self.logger.debug("the best hyperopt parameters: %s" % str(best_params))

        best_configuration = trials.results[np.argmin(trials.losses())]['train_command']
        best_loss = trials.results[np.argmin(trials.losses())]['loss']
        self.logger.info("\n\nA FULL TRAINING COMMAND WITH THE BEST HYPERPARAMETERS: \n%s" % best_configuration)
        self.logger.info("\n\nTHE BEST LOSS VALUE: \n%s" % best_loss)

        return best_configuration, best_loss


def main():
    args = read_arguments()
    h = HyperOptimizer(train_set=args.train, holdout_set=args.holdout, command=args.vw_space,
                       max_evals=args.max_evals,
                       outer_loss_function=args.outer_loss_function,
                       searcher=args.searcher, is_regression=args.regression)
    h.get_y_true_holdout()
    h.hyperopt_search()


if __name__ == '__main__':
    main()