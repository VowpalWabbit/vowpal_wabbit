from vowpalwabbit import pyvw

def get_latests_tests():
    import runtests_parser as rtp
    tests = rtp.file_to_obj(rtp.find_runtest_file())
    return [x.__dict__ for x in tests]


def get_all_options():
    return pyvw.get_all_vw_options()


def get_config_of_vw_cmd(test):
    vw = pyvw.vw(arg_str=test["vw_command"])
    config = vw.get_config()
    vw.finish()
    return config


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
    for name, config_group in config.items():
        for (group_name, options) in config_group:
            for option in options:
                if not option.value_supplied:
                    if len(config_group) <= 1:
                        print(name + ": " + option.name)
                    else:
                        print(name + ": " + group_name + ": " + option.name)
        

def main():
    allConfig = get_all_options() 
    tests = get_latests_tests()
    for test in tests:
        # fails for unknown reasons (possibly bugs with pyvw)
        if test["id"] in [195, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 258, 269]:
            continue

        if "vw_command" in test:
            config = get_config_of_vw_cmd(test)
            allConfig = merge_config(allConfig, config)
    
    print_non_supplied(allConfig)


if __name__ == "__main__":
    main()
