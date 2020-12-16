from vowpalwabbit import pyvw

import pytest

def test_vw_config_manager():
    vw = pyvw.vw(arg_str="--save_resume --loss_function logistic -d /root/vowpal_wabbit/test/train-sets/rcv1_small.dat --quiet")
    config = vw.get_config()

    cmd_str_list = []

    for name, config_group in config.items():
        for (group_name, options) in config_group:
            for option in options:
                temp_str = str(option)
                if temp_str:
                    cmd_str_list.append(temp_str)

    expected_set = {'--quiet', '--loss_function logistic', '--save_resume', '--data /root/vowpal_wabbit/test/train-sets/rcv1_small.dat'}
    assert set(cmd_str_list) == expected_set

    vw.finish()
