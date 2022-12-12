# Command Line for CSV Dataset 

This tutorial demonstrates how to approach a classification problem from the iris CSV dataset with Vowpal Wabbit. It features an overview of training and testing your model based on the dataset in CSV format, introduces Vowpal Wabbit CSV parsing features, and explains how to structure input and get the results.

For command line basics and more advanced Vowpal Wabbit tutorials, including understanding the results, the CSV dataset mostly shares the same methods as the VW format. See [Tutorials](https://vowpalwabbit.org/tutorials.html) for more.

```{admonition} Prerequisites
1. To install Vowpal Wabbit see [Get Started](https://vowpalwabbit.org/start.html).

2. You can get the CSV iris dataset from [here](https://github.com/VowpalWabbit/vowpal_wabbit/files/9203605/iris.csv).
```

## Training scenario and dataset

For this tutorial scenario, we want to use Vowpal Wabbit to help us classify iris plants into three species using the CSV format dataset. The dataset includes three iris species with 50 samples each as well as some properties of each flower.

First, download the CSV iris dataset from the internet:

```sh
wget https://github.com/VowpalWabbit/vowpal_wabbit/files/9203605/iris.csv -o iris.csv
```

The `iris.csv` is what we are going to use in the following steps, and it looks like this:

```csv
"","Sepal.Length","Sepal.Width","Petal.Length","Petal.Width","Species"
"1",5.1,3.5,1.4,0.2,"setosa"
"2",4.9,3,1.4,0.2,"setosa"
"3",4.7,3.2,1.3,0.2,"setosa"
"4",4.6,3.1,1.5,0.2,"setosa"
"5",5,3.6,1.4,0.2,"setosa"
"6",5.4,3.9,1.7,0.4,"setosa"
"7",4.6,3.4,1.4,0.3,"setosa"
"8",5,3.4,1.5,0.2,"setosa"
"9",4.4,2.9,1.4,0.2,"setosa"
......
```

which corresponds to the table:

| | Sepal.Length | Sepal.Width | Petal.Length | Petal.Width | Species |
| --- | --- | --- | --- | --- | --- |
1 | 5.1 | 3.5 | 1.4 | 0.2 | setosa |
2 | 4.9 | 3 | 1.4 | 0.2 | setosa |
3 | 4.7 | 3.2 | 1.3 | 0.2 | setosa |
4 | 4.6 | 3.1 | 1.5 | 0.2 | setosa |
5 | 5 | 3.6 | 1.4 | 0.2 | setosa |
6 | 5.4 | 3.9 | 1.7 | 0.4 | setosa |
7 | 4.6 | 3.4 | 1.4 | 0.3 | setosa |
8 | 5 | 3.4 | 1.5 | 0.2 | setosa |
9 | 4.4 | 2.9 | 1.4 | 0.2 | setosa |
. | ... | ... | ... | ...| ... |

As we can see from above, the first column is the row id ([tag](https://github.com/VowpalWabbit/vowpal_wabbit/wiki/Input-format)), the last column is what we are going to predict ([label](https://github.com/VowpalWabbit/vowpal_wabbit/wiki/Input-format)), and the other columns are features.

>**Note:** You can get a deeper understanding of CSV format from [Wikipedia](https://en.wikipedia.org/wiki/Comma-separated_values).

## Train a model

Next, we train a model and save it to a file.

### With full parameters
```sh
vw --csv -d iris.csv --csv_separator "," --csv_header "_tag,Sepal|Length,Sepal|Width,Petal|Length,Petal|Width,_label" --csv_ns_value Sepal:1,Petal:1 --named_labels setosa,versicolor,virginica --oaa 3 -f model.vw
```

This tells Vowpal Wabbit to:
- Specify the inputted format as `--csv` and `-d` **data** file as `iris.csv`.
- Use comma (`,`) as `--csv_separator`.
- Use args of `--csv_header` (`_tag,Sepal|Length,Sepal|Width,Petal|Length,Petal|Width,_label`) to replace the header in the inputted CSV dataset. `_label` to tell which is the label column, `_tag` to tell which is the tag column. Here we must use comma (`,`) as the separator for the args string, no matter what the `--csv_separator` is. `|` separates [namespace](https://github.com/VowpalWabbit/vowpal_wabbit/wiki/Namespaces) and feature names.
- Scale the namespace values by the ratio specified in the args of `--csv_ns_value` (`Sepal:1,Petal:1`, in format of `namespace1:ratio1,namespace2:ratio2,...`).
- Use `--named_labels`: `setosa`, `versicolor` and `virginica`.
- Consider the label as `--oaa` **one against all** with `3` classes.
- Write the `-f` **final** model to `model.vw`.

With Vowpal Wabbit, the output includes more than a few statistics and statuses. The [Linear Regression Tutorial](cmd_linear_regression.md#vowpal-wabbit-diagnostic-header) and [Contextual Bandit Reinforcement Learning Tutorial](python_Contextual_bandits_and_Vowpal_Wabbit.ipynb) cover this format in more detail:

Output:

```text
parsed 3 named labels
final_regressor = model.vw
using no cache
Reading datafile = iris.csv
num sources = 1
Num weight bits = 18
learning rate = 0.5
initial_t = 0
power_t = 0.5
Enabled reductions: gd, scorer-identity, oaa
Input label = MULTICLASS
Output pred = MULTICLASS
average  since         example        example        current        current  current
loss     last          counter         weight          label        predict features
0.000000 0.000000            1            1.0         setosa         setosa        5
0.000000 0.000000            2            2.0         setosa         setosa        5
0.000000 0.000000            4            4.0         setosa         setosa        5
0.000000 0.000000            8            8.0         setosa         setosa        5
0.000000 0.000000           16           16.0         setosa         setosa        5
0.000000 0.000000           32           32.0         setosa         setosa        5
0.031250 0.062500           64           64.0     versicolor     versicolor        5
[info] label 3 found -- labels are now considered 1-indexed.
0.039062 0.046875          128          128.0      virginica      virginica        5

finished run
number of examples = 150
weighted example sum = 150.000000
weighted label sum = 0.000000
average loss = 0.033333
total feature number = 750
```
### Fewer parameters

Since:
- Comma (`,`) is the default value for `--csv_separator`.
- We can modify the CSV file directly to avoid the `--csv_header` replacement.
- The default ratio for all namespaces is `1`.

Then we can have a shorter command to train the CSV dataset if we have modified the CSV dataset header in advance:

```csv
_tag,Sepal|Length,Sepal|Width,Petal|Length,Petal|Width,_label
"1",5.1,3.5,1.4,0.2,"setosa"
"2",4.9,3,1.4,0.2,"setosa"
......
```

The command is as follows, and the output should be the same with full parameters:

```sh
vw --csv -d iris.csv --named_labels setosa,versicolor,virginica --oaa 3 -f model.vw
```


## Test a model

Now, create a file called `test.csv` and copy this data:

```
5.1,3.8,1.5,0.3
```

We get a prediction by loading the model and supplying our test data:

```sh
vw --csv -d test.csv --csv_header "Sepal|Length,Sepal|Width,Petal|Length,Petal|Width" --csv_no_file_header --named_labels setosa,versicolor,virginica --oaa 3 -i model.vw -p predictions.txt
```

Aside from the ones that are already explained in the [Training section](#with-full-parameters), This tells Vowpal Wabbit to:
- Also consider the first line as the data with `--csv_no_file_header`, and thus must provide the `--csv_header` args.
- Use the `-i` **input** model `model.vw`.
- Write `-p` **predictions** to `predictions.txt`.

Output:

```text
parsed 3 named labels
predictions = predictions.txt
using no cache
Reading datafile = test.csv
num sources = 1
Num weight bits = 18
learning rate = 0.5
initial_t = 150
power_t = 0.5
Enabled reductions: gd, scorer-identity, oaa
Input label = MULTICLASS
Output pred = MULTICLASS
average  since         example        example        current        current  current
loss     last          counter         weight          label        predict features
[warning] No '_label' column found in the header/CSV first line!
n.a.     n.a.                1            1.0        unknown         setosa        5

finished run
number of examples = 1
weighted example sum = 1.000000
weighted label sum = 0.000000
average loss = n.a.
total feature number = 5
```

Kindly ignore the warnings as we are predicting without providing the label.

`cat predictions.txt` shows the iris classification based on the features we just provided:

```
setosa
```
