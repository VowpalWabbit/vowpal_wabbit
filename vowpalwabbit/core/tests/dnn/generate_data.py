import numpy as np
import configparser
import argparse
import os
import random

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

def generate_vw_cb_adf_data_adjusted(num_contexts, num_actions, context_feature_distribution, action_feature_distribution, file_name):
    def generate_features(feature_distribution):
        # Generate features according to the provided namespace distribution
        features = []
        for ns, num_features in feature_distribution.items():
            features.append(f"|{ns} ")
            for i in range(num_features):
                feature_name = f"f_{i}"
                feature_value = random.random()
                features.append(f"{feature_name}:{feature_value}")
        return " ".join(features)

    # Generate data
    data = []
    for _ in range(num_contexts):
        context_features = generate_features(context_feature_distribution)
        context_line = f"shared {context_features}"
        action_lines = []
        chosen_action = random.randint(1, num_actions)  # Randomly chosen action for cost and probability
        for action in range(1, num_actions + 1):
            action_features = generate_features(action_feature_distribution)
            if action == chosen_action:
                cost = random.randint(0, 1)  # Random cost (0 or 1 for simplicity)
                probability = random.uniform(0.1, 1.0)  # Random probability
                action_line = f"{action}:{cost}:{probability:.4f} {action_features}"
            else:
                action_line = f" {action_features}"
            action_lines.append(action_line)
        data.append("\n".join([context_line] + action_lines))

    # Write data to file
    print(f'Writing data to file ({file_name})...')
    with open(file_name, 'w') as f:
        f.write("\n\n".join(data))
    print('Done!')

if __name__ == '__main__':
    # Define command-line arguments
    parser = argparse.ArgumentParser()
    parser.add_argument('--config', type=str, default='config.ini', help='Path to configuration file')
    args = parser.parse_args()
    # Read configuration file
    config = configparser.ConfigParser()
    try:
        config.read(args.config)
        data_type = config.get('data', 'type')
        if data_type == 'regression':
            num_samples = config.getint(data_type, 'num_samples')
            num_features = config.getint(data_type, 'num_features')
            noise_stddev = config.getfloat(data_type, 'noise_stddev')
            file_name = config.get(data_type, 'file_name')
            create_vw_data_file(num_samples, num_features, noise_stddev, file_name)
        elif data_type == 'cb_adf':
            num_contexts = config.getint(data_type, 'num_contexts')
            num_actions = config.getint(data_type, 'num_actions')
            context_feature_distribution = eval(config.get(data_type, 'context_feature_distribution'))
            action_feature_distribution = eval(config.get(data_type, 'action_feature_distribution'))
            file_name = config.get(data_type, 'file_name')  # Get file name from config
            generate_vw_cb_adf_data_adjusted(num_contexts, num_actions, context_feature_distribution, action_feature_distribution, file_name)            
    except Exception as e:
        print(f'Error reading configuration file: {e}')
        # Print current working directory
        print(f'Current working directory: {os.getcwd()}')
        exit(1)