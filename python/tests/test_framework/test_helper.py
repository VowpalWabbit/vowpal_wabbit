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


def generate_mathematical_expression_json(config):
    expression = []
    variables_map = {}

    def add_exp(item):
        if isinstance(item, dict):
            plus = item.get("+", None)
            multiple = item.get("*", None)
            tmp = item.copy()
            if plus:
                del tmp["+"]
            if multiple:
                del tmp["*"]
            if tmp:
                expression.append("a" + str(len(variables_map)))
                variables_map["a" + str(len(variables_map))] = tmp

            if multiple:
                if expression and (expression[-1].isalnum() or expression[-1] == ")"):
                    expression.append("*")
                start_exp(multiple)
            if plus:
                if expression and (expression[-1].isalnum() or expression[-1] == ")"):
                    expression.append("+")
                expression.append("(")
                start_exp(plus)
                expression.append(")")
        elif isinstance(item, list):
            expression.append("(")
            start_exp(item)
            expression.append(")")

    def start_exp(item):
        for i in item:
            add_exp(i)

    start_exp(config)
    res = []
    ptr = 0
    while ptr < len(expression) - 1:
        if (
            expression[ptr].isalnum()
            and expression[ptr - 1] == "("
            and expression[ptr + 1] == ")"
        ):
            res.pop(-1)
            res.append(expression[ptr])
            ptr += 2
        else:
            res.append(expression[ptr])
            ptr += 1
    res.append(expression[-1])

    ptr = 0
    while ptr < len(res) - 1:
        if (
            res[ptr].isalnum()
            and (res[ptr + 1].isalnum() or res[ptr + 1] == "(")
            or (res[ptr] == ")" and res[ptr + 1].isalnum())
            or (res[ptr] == ")" and res[ptr + 1] == "(")
        ):
            res.insert(ptr + 1, "*")
            ptr += 2
        else:
            ptr += 1
    return "".join(res), variables_map


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
