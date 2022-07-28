from pathlib import Path
import subprocess

this_dir = Path(__file__).parent.resolve()

vw_bin:Path = this_dir / ".."/ ".." / ".."/ "build"/ "vowpalwabbit"/"cli"/"vw"
vw_merger_bin:Path = this_dir / ".."/ ".." / ".."/ "build"/ "vowpalwabbit"/"model_merger"/"vw-merge"

subprocess.check_call([vw_bin, "--driver_output_off", f"--data={this_dir/'data'/'0001.part1.vw'}", f"--final_regressor={this_dir / 'model1.vw'}"])
subprocess.check_call([vw_bin, "--driver_output_off", f"--data={this_dir/'data'/'0001.part2.vw'}", f"--final_regressor={this_dir / 'model2.vw'}"])
subprocess.check_call([vw_merger_bin, f"--output={this_dir / 'merged.vw'}", this_dir / 'model1.vw',  this_dir / 'model2.vw'])

subprocess.check_call([vw_bin, "--driver_output_off", "--no_stdin", "--preserve_performance_counters", f"--initial_regressor={this_dir/'model1.vw'}", f"--readable_model={this_dir / 'model1.vw.txt'}"])
subprocess.check_call([vw_bin, "--driver_output_off", "--no_stdin", "--preserve_performance_counters", f"--initial_regressor={this_dir/'model2.vw'}", f"--readable_model={this_dir / 'model2.vw.txt'}"])
subprocess.check_call([vw_bin, "--driver_output_off", "--no_stdin", "--preserve_performance_counters", f"--initial_regressor={this_dir/'merged.vw'}", f"--readable_model={this_dir / 'merged.vw.txt'}"])
