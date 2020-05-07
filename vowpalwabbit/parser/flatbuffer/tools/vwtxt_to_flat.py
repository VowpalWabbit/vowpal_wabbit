import flatbuffers
import argparse

from Label import *
from Namespace import *
from Feature import *
from ExampleCollection import *
from Example import *

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-d','--data',help="Data file", default=None)

    args = parser.parse_args()

    if args.data is None:
        raise FileNotFoundError
    else:
        with open(args.data, 'r') as file:
            data = file.read()

        builder = flatbuffers.Builder(1024)

        data = data.split('\n')
        examples = []
        for line in data:
            labels, namesfeatures = line.split('|')[0], line.split('|')[1:]
            LabelStart(builder)
            for i, label in enumerate(labels.split(' ')):
                if i==0:
                    LabelAddLabel(builder, int(label))
                if i==1:
                    if label == "":
                        break
                    LabelAddWeight(builder, float(label))

            label = LabelEnd(builder)

            if '|' in namesfeatures:
                namesfeatures = namesfeatures.split('|')
            else:
                namesfeatures = [namesfeatures]

            ns_flats = []
            for i, namespace in enumerate(namesfeatures):
                for j, nsfeature in enumerate(namespace.split(' ')):
                    FeatureStart(builder)
                    if j==0:
                        if nsfeature == '':
                            ns = ' '
                        else:
                            ns = nsfeature
                    else:
                        if ':' in nsfeature:
                            name, value = nsfeature.split(':')  
                        else:
                            name = nsfeature
                            value = 1.0 
                        FeatureAddName(builder, name)
                        FeatureAddValue(builder, value)
                        features.append(FeatureEnd(builder))
                    NamespaceStart(builder)
                    NamespaceAddName(builder, ns)
                    for ft in features:
                        NamespaceAddFeatures(builder, ft)
                ns_flats.append(NamespaceEnd)

            ExampleStart(builder)
            ExampleAddLabel(builder, label)
            for ns in ns_flats:
                ExampleAddNamespace(builder, ns)
            
            examples.append(ExampleEnd(builder))
        
        ExampleCollectionStart(builder)
        for eg in examples:
            ExampleCollectionAddExamples(builder, eg)
        
        egcollection = ExampleCollectionEnd(builder)

        egcollection.Finish()

                


if __name__ == "__main__":
    main()
