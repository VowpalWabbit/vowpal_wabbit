#!/usr/bin/env python3

"""
Utility script to replay DecisionService dsjson logs from a known initial model to obtain a given final target model.
"""

import os, hashlib, struct, argparse, subprocess

def check_result(result):
  if(result.returncode != 0):
    print(result.args)
    print(result.stderr)
    print(result.stdout)
    exit(1)

def create_readable_model(model_file, output_dir, vw_bin, empty_file):
    path, filename = os.path.split(model_file)
    readable_file_name = os.path.join(output_dir, filename + '.readable')
    cmd_str = vw_bin + ' -d ' + empty_file + ' -i ' + model_file + ' --readable_model ' + readable_file_name
    print('\nRUNNING: {}'.format(cmd_str))
    check_result(subprocess.run(cmd_str.split(), stdout=subprocess.PIPE, stderr=subprocess.STDOUT))
    return readable_file_name

def get_model_options(readable_model_file):
    with open(readable_model_file) as readable_model_file_handle:
        for line in readable_model_file_handle:
            if line.startswith("options:"):
                return line[8:].strip()

def get_model_id(model_fp):
    with open(model_fp, 'rb') as f:
        f.read(struct.unpack('I', f.read(4))[0])
        return f.read(struct.unpack('I', f.read(4))[0]).strip(b'\x00')

def build_combined_log(combined_log_file, log_files, start_after_event_id, final_event_id):
    count = 0
    stop = False
    print(start_after_event_id)
    print(final_event_id)
    with open(combined_log_file, 'wb') as file_out:
        found_first_event = False
        for log_file in log_files:
            print("Reading: " + str(log_file))
            i = 0
            for x in open(log_file, 'rb'):
                if found_first_event:
                    file_out.write(x)
                    count += 1
                    if b'"' + final_event_id + b'"' in x:
                        print('Found end event id at line: {} in {}. Stop!'.format(i, log_file))
                        stop = True
                        break
                else:
                    if b'"' + start_after_event_id + b'"' in x:
                        found_first_event = True
                        print('Found initial event id at line: {} in {}. Start!'.format(i, log_file))
                i += 1
            if stop:
                break

        if not stop:
            print("Didn't find both start and stop event ids! Stopping")
            os.remove(combined_log_file)
            exit()

    print('Number of train events: {}'.format(count))


def get_file_hash(file_name):
  return hashlib.sha256(open(file_name, 'rb').read()).hexdigest()

def replay_logs(vw_binary, combined_log_file, initial_model, final_model, reproduced_model_file, ml_args):
  final_model_id = get_model_id(final_model)
  if not os.path.isfile(reproduced_model_file):
    cmd_str = vw_binary + ' -i ' + initial_model + ' -d ' + combined_log_file + ' --dsjson -f ' + reproduced_model_file + ' --save_resume --preserve_performance_counters ' + ml_args + ' --id ' + final_model_id.decode() + ' -P 10000'
    print('\nRUNNING: {}'.format(cmd_str))
    check_result(subprocess.run(cmd_str.split(), stdout=subprocess.PIPE, stderr=subprocess.STDOUT))

def run_test_model_reproducibility(initial_model, final_model, log_files, vw_bins, output_dir):

    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    # Ensure the empty file used to create readable models exists
    empty_file_name = os.path.join(output_dir, 'empty.json')
    if not os.path.isfile(empty_file_name):
        open(empty_file_name, 'a').close()

    final_readable_model_file = create_readable_model(final_model, output_dir, vw_bins[0], empty_file_name)

    # Extract event ids used from model files
    initial_model_first_id, initial_model_last_id = get_model_id(initial_model).split(b'/')
    final_model_first_id, final_model_last_id = get_model_id(final_model).split(b'/')

    combined_log_file = os.path.join(output_dir, 'combined_log_file.json')
    if not os.path.isfile(combined_log_file):
        build_combined_log(combined_log_file, log_files, initial_model_last_id, final_model_last_id)

    results = []
    results.append((get_file_hash(final_readable_model_file), final_readable_model_file))

    # Obtain args from model file
    ml_args = get_model_options(final_readable_model_file)

    i = 0
    for v in vw_bins:
        reproduced_model_file = os.path.join(output_dir, str(i) + ".reproduced.vw")
        replay_logs(v, combined_log_file, initial_model, final_model, reproduced_model_file, ml_args)
        reproduced_readable_file = create_readable_model(reproduced_model_file, output_dir, vw_bins[0], empty_file_name)
        results.append((get_file_hash(reproduced_readable_file), reproduced_readable_file))
        i += 1

    print('\nSha256 of Original Model:')
    print('{} <- {}'.format(results[0][0], results[0][1]))
    print('\nSha256 of Generated Models files:')
    for x in results[1:]:
        print('{}: {} <- {}'.format(x[0], 'O' if x[0] == results[0][0] else 'X', x[1]))


if __name__ == '__main__':
  parser = argparse.ArgumentParser("replay_dsjson_logs")
  parser.add_argument("--initial_model", help="Model to start from", required=True)
  parser.add_argument("--final_model", help="Final model that should be reproduced", required=True)
  parser.add_argument('--log_files', nargs='+', help='dsjson logs to replay', required=True)
  parser.add_argument('--vw_bins', nargs='+', help='vw_binaries to test against', required=True)
  parser.add_argument("--output_dir", help="Directory to output files in", default=os.getcwd())
  args = parser.parse_args()

  initial_model = os.path.realpath(args.initial_model)
  final_model = os.path.realpath(args.final_model)
  output_dir = os.path.realpath(args.output_dir)
  log_files = [os.path.realpath(x) for x in args.log_files]
  vw_bins = [os.path.realpath(x) for x in args.vw_bins]

  run_test_model_reproducibility(initial_model, final_model, log_files, vw_bins, output_dir)
