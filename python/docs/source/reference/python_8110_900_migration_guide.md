# Python 8.11 to 9 migration

You can do nothing and everything should still work when migrating to VW 9. But if you want to move to the new way to avoid deprecation warnings and future breaks here's a rundown.

## Imports

Modules are now direcly accessible from the root module. In the past `from vowpalwabbit import ...` was needed, but now `import vowpalwabbit` and accessing with `.` will work.

### `pyvw`

The old `pyvw` types are now available under the root module.

Instead of:
```py
from vowpalwabbit import pyvw
```

Use:
```python
import vowpalwabbit
```

And use the corresponding types exposed by {py:mod}`vowpalwabbit` instead of {py:mod}`vowpalwabbit.pyvw`.

```{admonition} Exception
{py:class}`vowpalwabbit.pyvw.SearchTask` is only available under {py:mod}`vowpalwabbit.pyvw` due to its advanced nature and less usage.
```

### `DFtoVW`

Replace any reference to the old module name `DFtoVW` with {py:mod}`vowpalwabbit.dftovw`.

### `sklearn_vw`

Replace any reference to the old module name `sklearn_vw` with {py:class}`vowpalwabbit.sklearn`.

## Class names

- Replace {py:class}`vowpalwabbit.pyvw.vw` with {py:class}`vowpalwabbit.Workspace`
- Replace {py:class}`vowpalwabbit.pyvw.example` with {py:class}`vowpalwabbit.Example`
- Replace {py:class}`vowpalwabbit.pyvw.example_namespace` with {py:class}`vowpalwabbit.ExampleNamespace`
- Replace {py:class}`vowpalwabbit.pyvw.namespace_id` with {py:class}`vowpalwabbit.NamespaceId`
- Replace {py:class}`vowpalwabbit.pyvw.abstract_label` with {py:class}`vowpalwabbit.AbstractLabel`
- Replace {py:class}`vowpalwabbit.pyvw.simple_label` with {py:class}`vowpalwabbit.SimpleLabel`
- Replace {py:class}`vowpalwabbit.pyvw.multiclass_label` with {py:class}`vowpalwabbit.MulticlassLabel`
- Replace {py:class}`vowpalwabbit.pyvw.multiclass_probabilities_label` with {py:class}`vowpalwabbit.MulticlassProbabilitiesLabel`
- Replace {py:class}`vowpalwabbit.pyvw.cost_sensitive_label` with {py:class}`vowpalwabbit.CostSensitiveLabel`
- Replace {py:class}`vowpalwabbit.pyvw.cbandits_label` with {py:class}`vowpalwabbit.CBLabel`

## `get_prediction`

Instead of calling the free function {py:func}`vowpalwabbit.pyvw.get_prediction` call {py:meth}`vowpalwabbit.Example.get_prediction` on an {py:class}`vowpalwabbit.Example` instance.

## Label types and prediction types

- Instead of using the prediction type integer constants such as {py:const}`vowpalwabbit.pyvw.pylibvw.vw.pSCALAR` use the corresponding value in {py:class}`vowpalwabbit.PredictionType`
- Instead of using the label type integer constants such as {py:const}`vowpalwabbit.pyvw.pylibvw.vw.lSimple` use the corresponding value in {py:class}`vowpalwabbit.LabelType`
    - Instead of {py:const}`vowpalwabbit.pyvw.pylibvw.vw.lBinary` use {py:obj}`vowpalwabbit.LabelType.SCALAR`
    - Instead of {py:const}`vowpalwabbit.pyvw.pylibvw.vw.lDefault` use a value of `None`
    - {py:const}`vowpalwabbit.pyvw.pylibvw.vw.lMax` did not have a corresponding type

## Getting a label from an Example

- If using {py:meth}`vowpalwabbit.Example.get_label`, you should change the parameter from a class type to a {py:class}`vowpalwabbit.LabelType`.
- If calling {py:meth}`vowpalwabbit.AbstractLabel.from_example`. This has changed from a method on an existing instance to a static factory function which accepts an {py:class}`vowpalwabbit.Example` and returns an instance of the correspodning label type. Using {py:meth}`vowpalwabbit.Example.get_label` is preferred.

## ExampleNamespace.push_features

The first argument of {py:meth}`vowpalwabbit.ExampleNamespace.push_features` was removed as it was not needed and unused. Using two arguments is deprecated, to fix the callsites remove the first argument.
