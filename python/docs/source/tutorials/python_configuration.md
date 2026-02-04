# Configuring VW Models in Python

This tutorial explains how to configure Vowpal Wabbit models in Python, including how command-line arguments map to Python parameters.

```{admonition} Prerequisites
- Basic familiarity with Vowpal Wabbit concepts
- Python environment with `vowpalwabbit` installed
```

## Overview

VW is primarily configured through command-line-style options. The Python bindings translate these options into Python function parameters, allowing you to configure models in several equivalent ways.

## The Workspace Class

The `Workspace` class is the main entry point for using VW in Python. It accepts configuration in three forms that can be combined:

1. **Command-line string** (`arg_str`): A string of space-separated arguments
2. **Argument list** (`arg_list`): A list of string arguments
3. **Keyword arguments** (`**kwargs`): Python keyword arguments

### Basic Examples

```python
from vowpalwabbit import Workspace

# Method 1: Command-line string
vw1 = Workspace("--oaa 10 --bit_precision 24")

# Method 2: Keyword arguments
vw2 = Workspace(oaa=10, bit_precision=24)

# Method 3: Argument list
vw3 = Workspace(arg_list=["--oaa", "10", "--bit_precision", "24"])

# Method 4: Mixed (all three are merged)
vw4 = Workspace("--oaa 10", bit_precision=24)
```

All four examples above create equivalent configurations.

## How Command-Line Arguments Map to Python

### Naming Convention

Command-line options use dashes (`--learning-rate`), while Python uses underscores (`learning_rate`):

| Command Line | Python Keyword |
|--------------|----------------|
| `--learning_rate 0.5` | `learning_rate=0.5` |
| `--bit_precision 24` | `bit_precision=24` |
| `-b 24` | `b=24` |
| `--oaa 10` | `oaa=10` |

### Single-Character Options

Single-character options use a single dash on the command line but work the same way in Python:

```python
# These are equivalent:
vw = Workspace("-b 24 -l 0.1")
vw = Workspace(b=24, l=0.1)
```

### Boolean Flags

Boolean flags that don't take a value become `True`/`False` in Python:

```python
# Command line: vw --quiet --audit
vw = Workspace(quiet=True, audit=True)

# False values are omitted from the command line
vw = Workspace(quiet=True, audit=False)  # Only --quiet is passed
```

### Options with Multiple Values

Some options can be specified multiple times. Use a list in Python:

```python
# Command line: vw -q ab -q cd --interactions abc
vw = Workspace(q=["ab", "cd"], interactions=["abc"])

# Or using arg_list for full control:
vw = Workspace(arg_list=["-q", "ab", "-q", "cd", "--interactions", "abc"])
```

### Options with Spaces or Special Characters

When option values contain spaces or special characters, use `arg_list` instead of `arg_str`:

```python
# WRONG: arg_str splits naively on spaces
vw = Workspace("--data 'my file.txt'")  # Won't work correctly

# CORRECT: Use arg_list for precise control
vw = Workspace(arg_list=["--data", "my file.txt"])
```

## Common Configuration Patterns

### Multiclass Classification

```python
# One-Against-All with 10 classes
vw = Workspace(oaa=10, quiet=True)

# Error Correcting Tournament with 10 classes
vw = Workspace(ect=10, quiet=True)

# Cost-Sensitive One-Against-All
vw = Workspace(csoaa=10, quiet=True)
```

### Contextual Bandits

```python
# CB with ADF (action-dependent features)
vw = Workspace(cb_explore_adf=True, quiet=True)

# CB with epsilon-greedy exploration
vw = Workspace(cb_explore_adf=True, epsilon=0.1, quiet=True)
```

### Feature Engineering

```python
# Quadratic interactions between namespaces a and b
vw = Workspace(q=["ab"], quiet=True)

# Multiple quadratic interactions
vw = Workspace(q=["ab", "ac", "bc"], quiet=True)

# Cubic interactions
vw = Workspace(cubic=["abc"], quiet=True)

# General interactions syntax
vw = Workspace(interactions=["ab", "abc"], quiet=True)
```

### Regularization and Learning Rate

```python
vw = Workspace(
    learning_rate=0.5,   # or l=0.5
    l1=0.001,            # L1 regularization
    l2=0.001,            # L2 regularization
    power_t=0.5,         # Learning rate decay
    quiet=True
)
```

### Model Persistence

```python
# Save model after training
vw = Workspace(
    oaa=10,
    final_regressor="model.vw",  # or f="model.vw"
    quiet=True
)

# Load existing model
vw = Workspace(
    initial_regressor="model.vw",  # or i="model.vw"
    quiet=True
)
```

## Scikit-learn Wrapper

The `VW` class in `vowpalwabbit.sklearn` provides a scikit-learn compatible interface:

```python
from vowpalwabbit.sklearn import VW, VWClassifier, VWRegressor

# Basic usage with sklearn interface
model = VWClassifier(oaa=10, passes=3)
model.fit(X_train, y_train)
predictions = model.predict(X_test)

# All VW options are available as constructor parameters
model = VWRegressor(
    learning_rate=0.5,
    bit_precision=24,
    l2=0.001,
    passes=2
)
```

### Key sklearn Wrapper Parameters

| Parameter | Description |
|-----------|-------------|
| `convert_to_vw` | Auto-convert numpy arrays to VW format (default: True) |
| `convert_labels` | Convert [0,1] labels to [-1,1] (default: True) |
| `passes` | Number of training passes (default: 1) |
| `quiet` | Suppress VW output (default: True) |

## Inspecting Configuration

You can inspect the current configuration of a Workspace:

```python
vw = Workspace(oaa=10, learning_rate=0.5, quiet=True)

# Get all options
config = vw.get_config()
for opt in config:
    if opt.value_supplied:
        print(f"{opt.name}: {opt.value}")
```

## Common Pitfalls

### 1. Duplicate Options

When using multiple configuration methods, options are merged. Duplicates result in the option being specified multiple times:

```python
# This passes --oaa 5 --oaa 10 (might not be what you want)
vw = Workspace("--oaa 5", oaa=10)
```

### 2. Space-Splitting in arg_str

`arg_str` is split by spaces, which can cause issues:

```python
# WRONG: Splits into ["--data", "'my", "file.txt'"]
vw = Workspace("--data 'my file.txt'")

# CORRECT: Use arg_list
vw = Workspace(arg_list=["--data", "my file.txt"])
```

### 3. Boolean False Values

Setting a boolean option to `False` omits it entirely (doesn't pass `--no-option`):

```python
# This only passes --quiet, not --audit
vw = Workspace(quiet=True, audit=False)
```

### 4. Context Manager Usage

Always use context managers or call `finish()` to properly clean up:

```python
# Recommended: Use context manager
with Workspace(oaa=10, quiet=True) as vw:
    # Use vw
    pass  # Automatically cleaned up

# Alternative: Manual cleanup
vw = Workspace(oaa=10, quiet=True)
try:
    # Use vw
    pass
finally:
    vw.finish()
```

## Finding Available Options

To see all available VW options:

```bash
# From command line
vw --help
```

Or in Python:

```python
from vowpalwabbit import Workspace

# Create a workspace and inspect available options
vw = Workspace(quiet=True)
config = vw.get_config(filtered_enabled_reductions_only=False)
for opt in config:
    print(f"--{opt.name}: {opt.help_str}")
vw.finish()
```

## Summary

| Configuration Method | When to Use |
|---------------------|-------------|
| `arg_str` | Quick configuration with simple options |
| `arg_list` | When you need precise control over argument parsing |
| `**kwargs` | Most Pythonic, good for programmatic configuration |
| Mixed | Combine static base config with dynamic options |

The key mapping rules:
- Dashes become underscores: `--learning-rate` → `learning_rate`
- Boolean flags use `True`/`False`
- Multiple values use lists: `q=["ab", "cd"]`
- Single-char options work the same: `-b 24` → `b=24`
