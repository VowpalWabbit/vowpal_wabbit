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
            temp = labels.split(' ')
            temp = [i for i in temp if i is not '']
            if len(temp) == 1:
                LabelAddLabel(builder, float(temp[0]))
                print("Adding label {}".format(float(temp[0])))
                LabelAddWeight(builder, 1.0)
                print("Adding Weight {}".format(1.0))
            elif len(temp) == 2:
                LabelAddLabel(builder, float(temp[0]))
                print("Adding label {}".format(float(temp[0])))
                LabelAddWeight(builder, float(temp[1]))
                print("Adding Weight {}".format(float(temp[1])))
            # for i, label in enumerate(temp):
            #     if label == '':
            #         continue
            #     if i==0:
            #         LabelAddLabel(builder, float(label))
            #     if i==1:
            #         LabelAddWeight(builder, float(label))

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

        size = builder.Finish(egcollection)

        buffer = builder.Output()


        new = ExampleCollection.GetRootAsExampleCollection(buffer, 0)

        if new.ExamplesIsNone():
            print("Nothing in here")

        print(new.Examples(0).Label().Label())


        with open('test.dat','wb') as file:
            file.write(buffer)

        examples = ExampleCollection.GetRootAsExampleCollection(buffer, 0)

        print(size)

                
if __name__ == "__main__":
    main()
