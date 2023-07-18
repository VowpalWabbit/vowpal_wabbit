import json
import importlib
import os
import itertools
import inspect

# Get the current directory
current_dir = os.path.dirname(os.path.abspath(__file__))


def json_to_dict_list(file):
    with open(current_dir + "/test_configs/" + file, "r") as file:
        # Load the JSON data
        return json.load(file)


def evaluate_expression(expression, variables):
    # Create a dictionary to hold the variable values
    variables_dict = {}
    # Populate the variables_dict with the provided variables
    for variable_name, variable_value in variables.items():
        variables_dict[variable_name] = variable_value
    # Evaluate the expression using eval()
    result = eval(expression, variables_dict)
    return result


def variable_mapping(grids):
    variables_map = {}
    for i in range(len(grids)):
        variables_map["g" + str(len(variables_map))] = grids[i]
    return variables_map


def dynamic_function_call(module_name, function_name, *args, **kwargs):
    try:
        calling_frame = inspect.stack()[1]
        calling_module = inspect.getmodule(calling_frame[0])
        calling_package = calling_module.__package__
        module = importlib.import_module(module_name, package=calling_package)
        function = getattr(module, function_name)
        result = function(*args, **kwargs)
        return result
    except ImportError:
        print(f"Module '{module_name}' not found.")
    except AttributeError:
        print(f"Function '{function_name}' not found in module '{module_name}'.")


def get_function_object(module_name, function_name):
    try:
        calling_frame = inspect.stack()[1]
        calling_module = inspect.getmodule(calling_frame[0])
        calling_package = calling_module.__package__
        module = importlib.import_module(module_name, package=calling_package)
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
