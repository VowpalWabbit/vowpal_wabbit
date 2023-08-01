import json
import importlib
import os
import itertools
import inspect
import shutil

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
        pass
    except AttributeError:
        pass


def get_function_object(module_name, function_name):
    function = None
    try:
        calling_frame = inspect.stack()[1]
        calling_module = inspect.getmodule(calling_frame[0])
        calling_package = calling_module.__package__
        module = importlib.import_module(module_name, package=calling_package)
        function = getattr(module, function_name)
        return function
    except ImportError:
        pass
    except AttributeError:
        pass


def generate_string_combinations(*lists):
    combinations = list(itertools.product(*lists))
    combinations = ["".join(combination) for combination in combinations]
    return combinations


def copy_file(source_file, destination_file):
    try:
        shutil.copy(source_file, destination_file)
        print(f"File copied successfully from '{source_file}' to '{destination_file}'.")
    except FileNotFoundError:
        print(f"Source file '{source_file}' not found.")
    except PermissionError:
        print(
            f"Permission denied. Unable to copy '{source_file}' to '{destination_file}'."
        )


def call_function_with_dirs(dirs, module_name, function_name, **kargs):

    for dir in dirs:
        try:
            data = dynamic_function_call(
                dir + module_name,
                function_name,
                **kargs,
            )
            if data:
                return data
        except Exception as error:
            if type(error) not in [ModuleNotFoundError]:
                raise error


def get_function_obj_with_dirs(dirs, module_name, function_name):
    obj = None
    for dir in dirs:
        try:
            obj = get_function_object(
                dir + module_name,
                function_name,
            )
            if obj:
                return obj
        except Exception as error:
            if type(error) not in [ModuleNotFoundError]:
                raise error
    if not obj:
        raise ModuleNotFoundError(
            f"Module '{module_name}' not found in any of the directories {dirs}."
        )


def calculate_similarity(word, string):
    # Calculate the similarity score between the string and the word
    score = 0
    for char in word:
        if char in string:
            score += 1
    return score


def custom_sort(word, strings):
    # Sort the list of strings based on their similarity to the word
    return sorted(
        strings, key=lambda string: calculate_similarity(word, string), reverse=True
    )
