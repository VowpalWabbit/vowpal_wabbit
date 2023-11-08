import subprocess
import re
import matplotlib.pyplot as plt

def run_vw(input_file, output_file, log_file):
    # Use the vw.exe binary in the working directory
    # 
    # vw_path = "vw.exe"
    # vw_path = "C:/s/projects/work/fix_vw/vw_dnn/out/build/windows-x64-debug-dnn/vowpalwabbit/cli/MinSizeRel/vw.exe"
    # vw_path = "C:/s/projects/work/fix_vw/vw_dnn/out/build/windows-x64-debug-dnn/vowpalwabbit/cli/Debug/vw.exe"
    vw_path = "C:/s/projects/work/fix_vw/vw_dnn/out/build/windows-x64-debug-dnn/vowpalwabbit/cli/MinSizeRel/vw.exe"
    
    # cmd = [vw_path, "--dnn", "-d", input_file, "-f", output_file, "--passes", "100", "--cache_file", "cache", "--coin"]
    # cmd = [vw_path, "--dnn", "-d", input_file, "--passes", "100", "--cache"]
    cmd = [vw_path, "--dnn", "-d", input_file, "--passes", "4", "--cache", "--mini_batch_size", "10", "-P", "1000"]
    
    # cmd = [vw_path, "-d", input_file]
    # cmd = [vw_path, "-d", input_file, "--passes", "2", "--cache"]
    
    print(f"VW path = {vw_path}")
    # Open log file for writing
    with open(log_file, 'w') as f:
        try:
            # Run command and capture stdout and stderr to log file
            subprocess.run(cmd, check=True, stdout=f, stderr=subprocess.STDOUT)
        except subprocess.CalledProcessError as e:
            print(e)
            
            
# Sample output:
# 4.600166 4.600166            1            1.0        -2.1448         0.0000        6
# 2.392248 0.184330            2            2.0         0.5478         0.1185        6
# ...
# 0.288651 0.095640           64           64.0         0.0718        -0.0385        6
def plot_data(output_file):
    # Read output from file
    with open(output_file, 'r', encoding='utf-8') as f:
        sample_output = f.read()

    # Regular expression to extract average loss and example counter
    pattern = re.compile(r"([\d\.]+)\s+[\d\.]+\s+(\d+)\s+\d+\.\d+\s+[\d\.\-]+\s+[\d\.\-]+\s+\d+")

    # Extract data
    matches = pattern.findall(sample_output)
    average_losses = [float(match[0]) for match in matches]
    iterations = [int(match[1]) for match in matches]

    # Plot data
    plt.figure(figsize=(10, 6))
    plt.plot(iterations, average_losses, marker='o', linestyle='-')
    # plt.xscale('log', base=2)  # Setting x-axis to log scale with base 2
    plt.xlabel('Iteration Number')
    plt.ylabel('Average Loss')
    plt.title('Average Loss vs Iteration Number')
    plt.grid(True, which="both", ls="--")
    plt.xticks(iterations, iterations)  # Display all iteration numbers on the x-axis
    plt.show()

if __name__ == '__main__':
    input_file = 'synthetic_data.txt'
    output_file = 'model.vw'
    log_file = 'output.txt'
    run_vw(input_file, output_file, log_file)
    # Read log file and write to stdout
    with open(log_file, 'r') as f:
        sample_output = f.read()
        print(sample_output)
    # Plot data 
    plot_data(log_file)