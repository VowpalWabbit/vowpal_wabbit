import argparse
import os
import re
import subprocess
import sys
import time

USE_ADF = True
USE_CS = False

VW = '/scratch/clear/abietti/.local/bin/vw'
if USE_CS:
    VW_DS_DIR = '/scratch/clear/abietti/cb_eval/vwshuffled_cs/'
    DIR_PATTERN = '/scratch/clear/abietti/cb_eval/res_cs/cbresults_{}/'
else:
    VW_DS_DIR = '/scratch/clear/abietti/cb_eval/vwshuffled/'
    DIR_PATTERN = '/scratch/clear/abietti/cb_eval/res/cbresults_{}/'
# VW_DS_DIR = '/bscratch/b-albiet/vwshuffled/'
# DIR_PATTERN = '/bscratch/b-albiet/cbresults_{}/'

rgx = re.compile('^average loss = (.*)$', flags=re.M)


def expand_cover(policies):
    algs = []
    for psi in [0, 0.01, 0.1, 1.0]:
        algs.append(('cover', policies, 'psi', psi))
        algs.append(('cover', policies, 'psi', psi, 'nounif', None))
        # algs.append(('cover', policies, 'psi', psi, 'nounifagree', None, 'agree_mellowness', 0.1))
        # algs.append(('cover', policies, 'psi', psi, 'nounifagree', None, 'agree_mellowness', 0.01))
    return algs

params_old = {
    'alg': [
        ('supervised',),
        ('epsilon', 0),
        ('epsilon', 0.02),
        ('epsilon', 0.05),
        ('epsilon', 0.1),
        ('epsilon', 0.05, 'nounifagree', None, 'agree_mellowness', 1.0),
        ('epsilon', 0.05, 'nounifagree', None, 'agree_mellowness', 1e-2),
        ('epsilon', 0.05, 'nounifagree', None, 'agree_mellowness', 1e-4),
        ('epsilon', 0.05, 'nounifagree', None, 'agree_mellowness', 1e-6),
        # agree
        ('epsilon', 1, 'nounifagree', None, 'agree_mellowness', 1.0),
        ('epsilon', 1, 'nounifagree', None, 'agree_mellowness', 1e-2),
        ('epsilon', 1, 'nounifagree', None, 'agree_mellowness', 1e-4),
        ('epsilon', 1, 'nounifagree', None, 'agree_mellowness', 1e-6),
        ('bag', 2),
        ('bag', 4),
        ('bag', 8),
        ('bag', 16),
        ('bag', 2, 'greedify', None),
        ('bag', 4, 'greedify', None),
        ('bag', 8, 'greedify', None),
        ('bag', 16, 'greedify', None),
        ] + expand_cover(1) + expand_cover(4) + expand_cover(8) + expand_cover(16),
    'learning_rate': [0.001, 0.003, 0.01, 0.03, 0.1, 0.3, 1.0, 3.0, 10.0],
    'cb_type': ['dr', 'ips', 'mtr'],
    }

params = {
    'alg': [
        ('epsilon', 0.05, 'nounifagree', None, 'agree_mellowness', 1.0),
        ('epsilon', 0.05, 'nounifagree', None, 'agree_mellowness', 1e-2),
        ('epsilon', 0.05, 'nounifagree', None, 'agree_mellowness', 1e-4),
        ('epsilon', 0.05, 'nounifagree', None, 'agree_mellowness', 1e-6),
        ('epsilon', 1, 'nounifagree', None, 'agree_mellowness', 1.0),
        ('epsilon', 1, 'nounifagree', None, 'agree_mellowness', 1e-2),
        ('epsilon', 1, 'nounifagree', None, 'agree_mellowness', 1e-4),
        ('epsilon', 1, 'nounifagree', None, 'agree_mellowness', 1e-6),
        ],
    'learning_rate': [0.001, 0.003, 0.01, 0.03, 0.1, 0.3, 1.0, 3.0, 10.0],
    'cb_type': ['dr', 'ips', 'mtr'],
    }

extra_flags = None
# extra_flags = ['--loss0', '9', '--loss1', '10', '--baseline']

def param_grid():
    grid = [{}]
    for k in params:
        new_grid = []
        for g in grid:
            for param in params[k]:
                gg = g.copy()
                gg[k] = param
                new_grid.append(gg)
        grid = new_grid

    return sorted(grid)


def ds_files():
    import glob
    return sorted(glob.glob(os.path.join(VW_DS_DIR, '*.vw.gz')))


def get_task_name(ds, params):
    did, n_actions = os.path.basename(ds).split('.')[0].split('_')[1:]
    did, n_actions = int(did), int(n_actions)

    task_name = 'ds:{}|na:{}'.format(did, n_actions)
    if len(params) > 1:
        task_name += '|' + '|'.join('{}:{}'.format(k, v) for k, v in sorted(params.items()) if k != 'alg')
    task_name += '|' + ':'.join([str(p) for p in params['alg'] if p is not None])
    return task_name


def process(ds, params, results_dir):
    print 'processing', ds, params
    did, n_actions = os.path.basename(ds).split('.')[0].split('_')[1:]
    did, n_actions = int(did), int(n_actions)

    cmd = [VW, ds, '-b', '24']
    for k, v in params.iteritems():
        if k == 'alg':
            if v[0] == 'supervised':
                cmd += ['--csoaa' if USE_CS else '--oaa', str(n_actions)]
            else:
                cmd += ['--cbify', str(n_actions)]
                if USE_CS:
                    cmd += ['--cbify_cs']
                if extra_flags:
                    cmd += extra_flags
                if USE_ADF:
                    cmd += ['--cb_explore_adf']
                assert len(v) % 2 == 0, 'params should be in pairs of (option, value)'
                for i in range(len(v) / 2):
                    cmd += ['--{}'.format(v[2 * i])]
                    if v[2 * i + 1] is not None:
                        cmd += [str(v[2 * i + 1])]
        else:
            if params['alg'][0] == 'supervised' and k == 'cb_type':
                pass
            else:
	        cmd += ['--{}'.format(k), str(v)]

    print 'running', cmd
    t = time.time()
    output = subprocess.check_output(cmd, stderr=subprocess.STDOUT)
    sys.stderr.write('\n\n{}, {}, time: {}, output:\n'.format(ds, params, time.time() - t))
    sys.stderr.write(output)
    pv_loss = float(rgx.findall(output)[0])
    print 'elapsed time:', time.time() - t, 'pv loss:', pv_loss

    return pv_loss


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='vw job')
    parser.add_argument('task_id', type=int, help='task ID, between 0 and num_tasks - 1')
    parser.add_argument('num_tasks', type=int)
    parser.add_argument('--task_offset', type=int, default=0,
                        help='offset for task_id in output filenames')
    parser.add_argument('--results_dir', default=DIR_PATTERN.format('agree01'))
    parser.add_argument('--name', default=None)
    parser.add_argument('--test', action='store_true')
    parser.add_argument('--flags', default=None, help='extra flags for cb algorithms')
    args = parser.parse_args()

    if args.name is not None:
        args.results_dir = DIR_PATTERN.format(args.name)

    if args.flags is not None:
        extra_flags = args.flags.split()
    grid = param_grid()
    dss = ds_files()
    tot_jobs = len(grid) * len(dss)

    if args.task_id == 0:
        if not os.path.exists(args.results_dir):
            os.makedirs(args.results_dir)
            import stat
            os.chmod(args.results_dir, os.stat(args.results_dir).st_mode | stat.S_IWOTH)
    else:
        while not os.path.exists(args.results_dir):
            time.sleep(1)
    if not args.test:
        fname = os.path.join(args.results_dir, 'loss{}.txt'.format(args.task_offset + args.task_id))
        done_tasks = set()
        if os.path.exists(fname):
            done_tasks = set([line.split()[0] for line in open(fname).readlines()])
        loss_file = open(fname, 'a')
    idx = args.task_id
    while idx < tot_jobs:
        ds = dss[idx / len(grid)]
        params = grid[idx % len(grid)]
        if args.test:
            print ds, params
        else:
            task_name = get_task_name(ds, params)
            if task_name not in done_tasks:
                try:
                    pv_loss = process(ds, params, args.results_dir)
                    loss_file.write('{} {}\n'.format(task_name, pv_loss))
                    loss_file.flush()
                    os.fsync(loss_file.fileno())
                except subprocess.CalledProcessError:
                    sys.stderr.write('\nERROR: TASK FAILED {} {}\n\n'.format(ds, params))
                    print 'ERROR: TASK FAILED', ds, params
        idx += args.num_tasks

    if not args.test:
        loss_file.close()
