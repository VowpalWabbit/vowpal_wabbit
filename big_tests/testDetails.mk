
# test groups

regression_group:	107.test 2.test ;

# individual tests

107.inData := ftrl-proximal/ftrl-proximal-train.prep
107.params := -k -d $(dataDir)/$(107.inData) -f 0001_ftrl.model --passes 10 --ftrl --ftrl_alpha 0.01 --ftrl_beta 0 --l1 2 --cache --holdout_off

2.deps := 107.test
2.inData := ftrl-proximal/ftrl-proximal-train.prep
2.params := -k -d $(dataDir)/$(2.inData) -f 0001_ftrl.model --passes 10 --ftrl --ftrl_alpha 0.01 --ftrl_beta 0 --l1 2 --cache --holdout_off
2.otherOutputs := 0001_ftrl.model
