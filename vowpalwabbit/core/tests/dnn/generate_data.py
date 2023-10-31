import numpy as np
import configparser
import argparse
import os

def generate_vw_data(num_samples, num_features, noise_stddev):
    # Generate random feature weights for our synthetic function
    true_weights = np.random.randn(num_features)
    # Generate random features
    features = np.random.randn(num_samples, num_features)
    # Synthesize target values (labels)
    labels = features.dot(true_weights) + noise_stddev * np.random.randn(num_samples)
    # Convert data to VW format
    vw_data = []
    for i in range(num_samples):
        features_str = ' '.join([f'f{j+1}:{features[i, j]:.4f}' for j in range(num_features)])
        vw_data.append(f'{labels[i]:.4f} | {features_str}')
    return vw_data

def create_vw_data_file(num_samples, num_features, noise_stddev, file_name):
    # Generate data
    vw_data = generate_vw_data(num_samples, num_features, noise_stddev)
    # Write data to file
    print(f'Writing data to file ({file_name})...')
    with open(file_name, 'w') as f:
        f.write('\n'.join(vw_data))
    print('Done!')
    return vw_data

if __name__ == '__main__':
    # Define command-line arguments
    parser = argparse.ArgumentParser()
    parser.add_argument('--config', type=str, default='config.ini', help='Path to configuration file')
    args = parser.parse_args()
    # Read configuration file
    config = configparser.ConfigParser()
    try:
        config.read(args.config)
        num_samples = config.getint('data', 'num_samples')
        num_features = config.getint('data', 'num_features')
        noise_stddev = config.getfloat('data', 'noise_stddev')
        file_name = config.get('output', 'file_name')
    except Exception as e:
        print(f'Error reading configuration file: {e}')
        # Print current working directory
        print(f'Current working directory: {os.getcwd()}')
        exit(1)
    # Generate data
    create_vw_data_file(num_samples, num_features, noise_stddev, file_name)
    # Show sample of the dataset
    print(f'Sample of the dataset:')
    vw_data = generate_vw_data(num_samples=100, num_features=5, noise_stddev=0.05)
    sample_data = '\n'.join(vw_data[:5])
    print(sample_data)