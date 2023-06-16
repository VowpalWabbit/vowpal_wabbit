import json
import importlib
import pytest
import os
import itertools

# Get the current directory
current_dir = os.path.dirname(os.path.abspath(__file__))


def json_to_dict_list(file):
    with open(current_dir + "/" + file, "r") as file:
        # Load the JSON data
        return json.load(file)


def dynamic_function_call(module_name, function_name, *args, **kwargs):
    try:
        module = importlib.import_module(module_name)
        function = getattr(module, function_name)
        result = function(*args, **kwargs)
        return result
    except ImportError:
        print(f"Module '{module_name}' not found.")
    except AttributeError:
        print(f"Function '{function_name}' not found in module '{module_name}'.")


def get_function_object(module_name, function_name):
    try:
        module = importlib.import_module(module_name)
        function = getattr(module, function_name)
        return function
    except ImportError:
        print(f"Module '{module_name}' not found.")
    except AttributeError:
        print(f"Function '{function_name}' not found in module '{module_name}'.")


def generate_string_combinations(*lists):
    combinations = list(itertools.product(*lists))
    combinations = ["".join(combination) for combination in combinations]
    return combinations
