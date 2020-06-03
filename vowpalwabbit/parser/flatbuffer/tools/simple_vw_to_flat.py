import flatbuffers
import argparse

from Label import *
from Namespace import *
from Feature import *
from ExampleCollection import *
from Example import *

builder = flatbuffers.Builder(1024)

LabelStart(builder)
LabelAddLabel(builder, 3.0)
LabelAddWeight(builder, 1.0)
label = LabelEnd(builder)

ft = builder.CreateString("feature")
FeatureStart(builder)
FeatureAddName(builder, ft)
FeatureAddValue(builder, 2.0)
ft = FeatureEnd(builder)

ns = builder.CreateString("namespace")
NamespaceStart(builder)
NamespaceAddName(builder, ns)
NamespaceAddFeatures(builder, ft)
ns = NamespaceEnd(builder)

ExampleStart(builder)
ExampleAddLabel(builder, label)
ExampleAddNamespaces(builder, ns)
eg = ExampleEnd(builder)

ExampleCollectionStart(builder)
ExampleCollectionAddExamples(builder, eg)
egcollection = ExampleCollectionEnd(builder)

print("Number of examples", egcollection)
size = builder.Finish(egcollection)
buffer = builder.Output()

new = ExampleCollection.GetRootAsExampleCollection(buffer, 0)

if new.ExamplesIsNone():
    print("Nothing in here")

print("Number of Examples {}".format(new.ExamplesLength()))
print("Weight: {}".format(new.Examples(0).Label().Weight()))
print("Label: {}".format(new.Examples(0).Label().Label()))
print("Feature Value: {}".format(new.Examples(7)))



# with open('test.dat','wb') as file:
#     file.write(buffer)

# examples = ExampleCollection.GetRootAsExampleCollection(buffer, 0)
