from vowpalwabbit import pyvw

def get_latest_tests():
    import runtests_parser as rtp
    tests = rtp.file_to_obj(rtp.find_runtest_file())
    return [x.__dict__ for x in tests]


def get_all_options():
    return pyvw.get_all_vw_options()


def get_config_of_vw_cmd(test):
    vw = pyvw.vw(arg_str=test["vw_command"])
    config = vw.get_config()
    enabled_reductions = vw.get_enabled_reductions()
    vw.finish()
    return config, enabled_reductions


def update_option(config, name, group_name, option_name):
    for (g_n, options) in config[name]:
        if g_n == group_name:
            for option in options:
                if option.name == option_name:
                    option.value = True

    return config


def merge_config(tracker, b):
    for name, config_group in b.items():
        for (group_name, options) in config_group:
            for option in options:
                if option.value_supplied:
                    tracker = update_option(tracker, name, group_name, option.name)

    return tracker


def print_non_supplied(config):
    with_default = []
    without_default = []

    for name, config_group in config.items():
        for (group_name, options) in config_group:
            for option in options:
                if not option.value_supplied:
                    default_val_str = ""
                    if option.default_value_supplied:
                        default_val_str = ", BUT has default value"
                        agg = with_default

                    if len(config_group) <= 1:
                        agg.append(name + ", " + option.name + default_val_str)
                    else:
                        agg.append(name + ", " + group_name + ", " + option.name + default_val_str)
                    
                    agg = without_default
    
    for e in with_default:
        print(e)
    for e in without_default:
        print(e)
        

def main():
    stacks = []

    allConfig = get_all_options() 
    tests = get_latest_tests()
    for test in tests:
        # fails for unknown reasons (possibly bugs with pyvw)
        if test["id"] in [195, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 258, 269]:
            continue

        if "vw_command" in test:
            config, enabled_reductions = get_config_of_vw_cmd(test)
            stacks.append('->'.join(enabled_reductions))
            allConfig = merge_config(allConfig, config)
    
    print_non_supplied(allConfig)

    # print reduction stack by count
    from collections import Counter
    for c in Counter(stacks).most_common():
        print(c[0]+", "+str(c[1]))


if __name__ == "__main__":
    main()
