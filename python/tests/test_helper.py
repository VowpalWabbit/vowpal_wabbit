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


def evaluate_expression(expression, variables):
    # Create a dictionary to hold the variable values
    variables_dict = {}

    # Populate the variables_dict with the provided variables
    for variable_name, variable_value in variables.items():
        variables_dict[variable_name] = variable_value

    # Evaluate the expression using eval()
    result = eval(expression, variables_dict)
    return result


def generate_mathematical_expression_json(config):
    expression = ""

    for i, item in enumerate(config):
        if isinstance(item, dict):
            if expression and (expression[-1].isdigit()):
                expression += " * "
            if expression and expression[-1] == ")":
                expression += " * "
            expression += "a" + str(config.index(item))

        elif isinstance(item, str):
            if item == "(":
                if expression and (expression[-1].isdigit() or expression[-1] == ")"):
                    expression += " * "
                expression += "("
            elif item == "+":
                expression += " + "
            elif item == ")":
                expression += ")"

    return expression


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
