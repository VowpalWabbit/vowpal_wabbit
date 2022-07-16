import argparse
from pathlib import Path
import subprocess
import json
import sys

import yaml


def main():
    parser = argparse.ArgumentParser(
        description="Extract commits from a git repo and format according to template"
    )
    parser.add_argument("--template", type=str, required=True)
    parser.add_argument(
        "--dump-options-bin",
        type=str,
        required=True,
    )
    parser.add_argument("--out-dir", type=str, required=True)
    parser.add_argument("--extra-info", type=str, required=True)
    args = parser.parse_args()

    with open(args.extra_info) as f:
        extra_info = yaml.load(f, Loader=yaml.Loader)

    resolved_out_dir = Path(args.out_dir).resolve()
    resolved_template = Path(args.template).resolve()

    result = subprocess.check_output([args.dump_options_bin])
    data = json.loads(result)

    used_extra_info_options = set()
    used_extra_info_groups = set()

    # enrich with extra_info
    for group in data["option_groups"]:
        if group["name"] in extra_info["groups"]:
            group["extra_info"] = extra_info["groups"][group["name"]]
            used_extra_info_groups.add(group["name"])

        for option in group["options"]:
            if option["name"] in extra_info["options"]:
                option["extra_info"] = extra_info["options"][option["name"]]
                used_extra_info_options.add(option["name"])

    unused_options = set(extra_info["options"].keys()).difference(
        used_extra_info_options
    )
    if len(unused_options) > 0:
        print(f"Error: extra_info['options'] keys not used: {unused_options}")
        sys.exit(1)

    unused_groups = set(extra_info["groups"].keys()).difference(used_extra_info_groups)
    if len(unused_groups) > 0:
        print(f"Error: extra_info['groups'] keys not used: {unused_groups}")
        sys.exit(1)

    base_dir = Path(__file__).resolve().parent
    cmd = [
        "hbs",
        "--stdin",
        "--helper",
        str(base_dir / "helpers") + "/*.js",
        "--partial",
        str(base_dir) + "/inner_template.hbs",
        "--extension",
        "rst",
        "--output",
        str(resolved_out_dir),
        "--",
        str(resolved_template),
    ]
    print(" ".join(cmd))
    with open("data.json", "w") as f:
        json.dump(data, f, indent=2)
    process = subprocess.Popen(cmd, stdin=subprocess.PIPE, cwd=base_dir)
    process.communicate(input=json.dumps(data).encode("utf-8"))
    if process.wait() != 0:
        raise Exception("hbs failed")


if __name__ == "__main__":
    main()
