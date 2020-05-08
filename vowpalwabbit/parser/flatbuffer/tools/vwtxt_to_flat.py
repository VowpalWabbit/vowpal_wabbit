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
                if label == '':
                    continue
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
                if isinstance(namesfeatures, list):
                    pass
                elif isinstance(namesfeatures, str):
                    namesfeatures = [namesfeatures]
                else:
                    raise NotImplementedError("Type {} not supported for {}".format(type(namesfeatures), namesfeatures))

            ns_flats = []
            for i, namespace in enumerate(namesfeatures):
                features = []
                for j, nsfeature in enumerate(namespace.split(' ')):
                    if j==0:
                        if nsfeature == '':
                            ns = ' '
                        else:
                            ns = nsfeature
                    else:
                        if ':' in nsfeature:
                            name, value = nsfeature.split(':')  
                            name = builder.CreateString(name)
                        else:
                            name = builder.CreateString(nsfeature)
                            value = 1.0
                        FeatureStart(builder)
                        FeatureAddName(builder, name)
                        FeatureAddValue(builder, float(value))
                        features.append(FeatureEnd(builder))
                ns = builder.CreateString(ns)
                NamespaceStart(builder)
                NamespaceAddName(builder, ns)
                for ft in features:
                    NamespaceAddFeatures(builder, ft)
                ns_flats.append(NamespaceEnd(builder))

            ExampleStart(builder)
            ExampleAddLabel(builder, label)
            for ns in ns_flats:
                ExampleAddNamespaces(builder, ns)
            
            examples.append(ExampleEnd(builder))
        
        ExampleCollectionStart(builder)
        for eg in examples:
            ExampleCollectionAddExamples(builder, eg)
        
        egcollection = ExampleCollectionEnd(builder)

        builder.Finish(egcollection)

        buffer = builder.Output()
        with open('test.dat','wb') as file:
            file.write(buffer)

                
if __name__ == "__main__":
    main()
