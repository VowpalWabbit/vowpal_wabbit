from vowpalwabbit import pyvw
import vowpalwabbit


def helper_options_to_list_strings(config):
    cmd_str_list = []

    for name, config_group in config.items():
        for (group_name, options) in config_group:
            for option in options:
                temp_str = str(option)
                if temp_str:
                    cmd_str_list.append(temp_str)

    return cmd_str_list


def test_vw_config_manager():
    expected_set = {
        "--no_stdin",
        "--quiet",
        "--loss_function=logistic",
        "--data=test/train-sets/rcv1_small.dat",
    }
    expected_reductions = {"gd", "scorer-identity", "count_label"}

    vw = vowpalwabbit.Workspace(
        arg_str="--loss_function logistic -d test/train-sets/rcv1_small.dat --quiet"
    )
    config = vw.get_config()
    enabled_reductions = vw.get_enabled_reductions()

    cmd_str_list = helper_options_to_list_strings(config)
    assert set(cmd_str_list) == expected_set
    assert set(enabled_reductions) == expected_reductions

    vw.finish()

    # do another iteration generating the cmd string from the output of previous
    new_args = " ".join(cmd_str_list)

    other_vw = vowpalwabbit.Workspace(new_args)
    new_config = vw.get_config()
    new_cmd_str_list = helper_options_to_list_strings(new_config)

    assert set(new_cmd_str_list) == expected_set

    other_vw.finish()


def test_vw_get_all_options():
    config = pyvw.get_all_vw_options()

    cmd_str_list = set()

    for name, config_group in config.items():
        cmd_str_list.add(name)

    assert len(cmd_str_list) >= 74


def test_experimental_option_value():
    has_experimental = False
    vw = vowpalwabbit.Workspace("--cb_explore_adf --automl 3", quiet=True)
    for groups in vw.get_config().values():
        for group in groups:
            for opt in group[1]:
                if opt.experimental:
                    has_experimental = True
                    break
        if has_experimental:
            break
    vw.finish()
    assert has_experimental
