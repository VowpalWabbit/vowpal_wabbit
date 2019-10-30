import sys
import argparse
import subprocess

if __name__ == '__main__':
  parser = argparse.ArgumentParser()
  parser.add_argument("--vw", help="Path to VW binary to use", type=str, required=True)
  parser.add_argument("--spanning_tree", help="Path to spanning tree binary to use", type=str, required=True)
  parser.add_argument("--data_files", help="Data files to use, one per node", type=str, nargs='+', required=True)
  parser.add_argument("--test_file", help="Test input file to feed to final model", type=str, required=True)
  parser.add_argument("--vw_args", help="Extra vw arguments to to use", type=str, default="")
  parser.add_argument("--prediction_file", help="", type=str, default=None)
  args = parser.parse_args()

  spanning_tree_args = [args.spanning_tree, "--nondaemon"]
  print("Starting spanning_tree with args: " + " ".join(spanning_tree_args[1:]))
  spanning_tree_proc = subprocess.Popen(spanning_tree_args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

  split_vw_args = args.vw_args.split()
  vw_procs = []
  for index, data_file in enumerate(args.data_files):
    cmd_args = [args.vw, "--span_server", "localhost", "--total", str(len(args.data_files)), "--node", str(index), "--unique_id", "1234", "-d", data_file]
    cmd_args.extend(split_vw_args)
    if(index == len(args.data_files) - 1):
      cmd_args.extend(["-f", "final.model"])
    print("Starting VW with args: " + " ".join(cmd_args[1:]))
    vw_procs.append(subprocess.Popen(cmd_args, stdout=subprocess.PIPE, stderr=subprocess.PIPE))

  for proc in vw_procs:
    return_code = proc.wait()
    print("VW succeeded")
    if return_code != 0:
      print("VW failed:")
      print("STDOUT: \n" + proc.stdout.read().decode("utf-8"))
      print("STDERR: \n" + proc.stderr.read().decode("utf-8"))
      spanning_tree_proc.kill()
      sys.exit(1)

  spanning_tree_proc.kill()

  cmd_args = [args.vw, "-d", args.test_file, "-i", "final.model", "-t"]
  cmd_args.extend(split_vw_args)
  if(args.prediction_file is not None):
    cmd_args.extend(["-p", args.prediction_file])
  print("Running test on produced model...")
  print("Running VW with args: " +  " ".join(cmd_args[1:]))

  cmd_args.extend(split_vw_args)
  subprocess.Popen(cmd_args).wait()
