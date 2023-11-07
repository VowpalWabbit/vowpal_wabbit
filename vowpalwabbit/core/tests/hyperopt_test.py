import subprocess
import os

# Test vw-hyperopt outer_loss_functions

# logistic
print("Checking outer loss function logistic")
try:
    subprocess.call(
        [
            "python ../../utl/vw-hyperopt.py  --train ../../test/train-sets/hyperopt_test.dat --holdout ../../test/train-sets/hyperopt_test.dat \
			--max_evals 1 --outer_loss_function logistic --vw_space '--algorithms=ftrl --ftrl_alpha=5e-5..5e-1~L --ftrl_beta=0.01..1 \
  				--loss_function=logistic ' "
        ],
        shell=True,
    )
    print("Done checking outer loss function logistic ")
except:
    print("Invalide command for outer loss function logistic")

# hinge
print("Checking outer loss function hinge")
try:
    subprocess.call(
        [
            "python ../../utl/vw-hyperopt.py  --train ../../test/train-sets/hyperopt_test.dat --holdout ../../test/train-sets/hyperopt_test.dat \
			--max_evals 1 --outer_loss_function hinge --vw_space '--algorithms=ftrl --ftrl_alpha=5e-5..5e-1~L --ftrl_beta=0.01..1 \
  				--loss_function=hinge ' "
        ],
        shell=True,
    )
    print("Done checking outer loss function hinge ")
except:
    print("Invalide command for outer loss function hinge")

# squared
print("Checking outer loss function squared")
try:
    subprocess.call(
        [
            "python ../../utl/vw-hyperopt.py  --train ../../test/train-sets/hyperopt_test.dat --holdout ../../test/train-sets/hyperopt_test.dat \
			--max_evals 1 --outer_loss_function squared --vw_space '--algorithms=ftrl --ftrl_alpha=5e-5..5e-1~L --ftrl_beta=0.01..1 \
  				--loss_function=squared ' "
        ],
        shell=True,
    )
    print("Done checking outer loss function squared ")
except:
    print("Invalide command for outer loss function squared")

# pr-auc
print("Checking outer loss function pr-auc")
try:
    subprocess.call(
        [
            "python ../../utl/vw-hyperopt.py  --train ../../test/train-sets/hyperopt_test.dat --holdout ../../test/train-sets/hyperopt_test.dat \
			--max_evals 1 --outer_loss_function pr-auc --vw_space '--algorithms=ftrl --ftrl_alpha=5e-5..5e-1~L --ftrl_beta=0.01..1 \
  				--loss_function=logistic ' "
        ],
        shell=True,
    )
    print("Done checking outer loss function pr-auc ")
except:
    print("Invalide command for outer loss function pr-auc")

# roc-auc
print("Checking outer loss function roc-auc")
try:
    subprocess.call(
        [
            "python ../../utl/vw-hyperopt.py  --train ../../test/train-sets/hyperopt_test.dat --holdout ../../test/train-sets/hyperopt_test.dat \
			--max_evals 1 --outer_loss_function roc-auc --vw_space '--algorithms=ftrl --ftrl_alpha=5e-5..5e-1~L --ftrl_beta=0.01..1 \
  				--loss_function=logistic ' "
        ],
        shell=True,
    )
    print("Done checking outer loss function roc-auc ")
except:
    print("Invalide command for outer loss function roc-auc")

# quantile
print("Checking outer loss function quantile")
try:
    subprocess.call(
        [
            "python ../../utl/vw-hyperopt.py  --train ../../test/train-sets/hyperopt_test.dat --holdout ../../test/train-sets/hyperopt_test.dat \
			--max_evals 1 --outer_loss_function quantile --vw_space '--algorithms=ftrl --ftrl_alpha=5e-5..5e-1~L --ftrl_beta=0.01..1 \
  				--loss_function=quantile ' "
        ],
        shell=True,
    )
    print("Done checking outer loss function quantile ")
except:
    print("Invalide command for outer loss function quantile")
