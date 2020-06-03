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

FeatureStart(builder)
FeatureAddName(builder, builder.CreateString("feature"))
FeatureAddValue(builder, 2.0)
ft = FeatureEnd(builder)

NamespaceStart(builder)
NamespaceAddName(builder, builder.CreateString("namespace"))
NamespaceAddFeatures(builder, ft)
ns = NamespaceEnd(builder)

ExampleStart(builder)
ExampleAddLabel(builder, label)
ExampleAddNamespaces(builder, ns)
eg = ExampleEnd(builder)
        

ExampleCollectionStart(builder)
ExampleCollectionAddExamples(builder, eg)
egcollection = ExampleCollectionEnd(builder)

size = builder.Finish(egcollection)
buffer = builder.Output()


new = ExampleCollection.GetRootAsExampleCollection(buffer, 0)

if new.ExamplesIsNone():
    print("Nothing in here")

print(new.Examples(0).Label().Label())

# with open('test.dat','wb') as file:
#     file.write(buffer)

# examples = ExampleCollection.GetRootAsExampleCollection(buffer, 0)
