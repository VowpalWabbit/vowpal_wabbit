import json
import importlib
import pytest
import os

# Get the current directory
current_dir = os.path.dirname(os.path.abspath(__file__))

def json_to_dict_list(file):
    res = []
    with open(current_dir + "/" + file, 'r') as file:
        # Load the JSON data
        json_data = json.load(file)
        # Iterate over each item in the array
        for item in json_data:
            # Convert item to a dictionary
            item_dict = dict(item)
            res.append(item_dict)
            # Process the dictionary as needed
    return res


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



def generate_test_function(test_data):
    @pytest.dynamic
    def test_dynamic():
        pass
        # Perform the test using the test_data
        # ...

    # Set a custom name for the test function
    test_dynamic.__name__ = test_data["name"]

    return test_dynamic


def generate_pytest_from_json(filepath):
    # Load the JSON data from a file
    with open(filepath, "r") as file:
        json_data = json.load(file)

    # Iterate over the JSON data and dynamically generate the test functions
    for test_case in json_data:
        test_function = generate_test_function(test_case)
        globals()[test_function.__name__] = test_function
    