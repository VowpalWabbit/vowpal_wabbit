from vowpalwabbit import pyvw
import json
import os
from pathlib import Path


def get_latest_tests(file_path=None):
    if file_path is None:
        test_ref_dir = Path(os.path.dirname(os.path.abspath(__file__)))
        file_path = Path(test_ref_dir).joinpath("core.vwtest.json")

    json_test_spec_content = open(file_path).read()
    tests = json.loads(json_test_spec_content)
    return [x.__dict__ for x in tests]


def get_all_options():
    return pyvw.get_all_vw_options()


def to_json():
    config = get_all_options()
    for name, config_group in config.items():
        for group_name, options in config_group:
            for option in options:
                option._type = str(type(option._default_value).__name__)

    import json

    with open("vw_options.json", "w") as f:
        f.write(json.dumps(config, indent=2, default=lambda x: x.__dict__))


def get_config_of_vw_cmd(test):
    vw = pyvw.Workspace(arg_str=test["vw_command"])
    config = vw.get_config()
    enabled_reductions = vw.get_enabled_reductions()
    vw.finish()
    return config, enabled_reductions


def update_option(config, name, group_name, option_name):
    for g_n, options in config[name]:
        if g_n == group_name:
            for option in options:
                if option.name == option_name:
                    option.value = True

    return config


def merge_config(tracker, b):
    for name, config_group in b.items():
        for group_name, options in config_group:
            for option in options:
                if option.value_supplied:
                    tracker = update_option(tracker, name, group_name, option.name)

    return tracker


def print_non_supplied(config):
    with_default = []
    without_default = []

    for name, config_group in config.items():
        for group_name, options in config_group:
            for option in options:
                if not option.value_supplied:
                    default_val_str = ""
                    if option.default_value_supplied:
                        default_val_str = ", BUT has default value"
                        agg = with_default

                    if len(config_group) <= 1:
                        agg.append(name + ", " + option.name + default_val_str)
                    else:
                        agg.append(
                            name
                            + ", "
                            + group_name
                            + ", "
                            + option.name
                            + default_val_str
                        )

                    agg = without_default

    for e in with_default:
        print(e)
    for e in without_default:
        print(e)


# this function needs more depedencies (networkx, matplotlib, graphviz)
def draw_graph(stacks):
    import networkx as nx
    import matplotlib.pyplot as plt
    from networkx.drawing.nx_agraph import write_dot, graphviz_layout

    G = nx.DiGraph()

    for l in stacks:
        reductions = l.split("->")
        for i, k in zip(reductions, reductions[1:]):
            G.add_edge(
                k.replace("cb_explore_adf_", ""), i.replace("cb_explore_adf_", "")
            )
        if len(reductions) == 1:
            G.add_node(reductions[0])

    plt.figure(num=None, figsize=(24, 12), dpi=120, facecolor="w", edgecolor="k")

    write_dot(G, "graphviz_format.dot")

    pos = graphviz_layout(
        G,
        prog="dot",
        args='-Nfontsize=10 -Nwidth=".2" -Nheight=".2" -Nmargin=0 -Gfontsize=12',
    )
    nx.draw(G, pos, with_labels=True, arrows=True, node_size=1600)
    plt.savefig("reduction_graph.png")


def main():
    stacks = []

    allConfig = get_all_options()
    tests = get_latest_tests()
    for test in tests:
        # fails for unknown reasons (possibly bugs with pyvw)
        if test["id"] in [
            195,
            236,
            237,
            238,
            239,
            240,
            241,
            242,
            243,
            244,
            245,
            246,
            258,
            269,
        ]:
            continue

        if "vw_command" in test:
            config, enabled_reductions = get_config_of_vw_cmd(test)
            stacks.append("->".join(enabled_reductions))
            allConfig = merge_config(allConfig, config)

    print_non_supplied(allConfig)

    # draw_graph(stacks)

    # print reduction stack by count
    from collections import Counter

    for c in Counter(stacks).most_common():
        print(c[0] + ", " + str(c[1]))


if __name__ == "__main__":
    main()
