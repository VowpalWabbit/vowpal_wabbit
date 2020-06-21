import argparse

import flatbuffers

from VW.parsers.flatbuffer import Label, Namespace, Feature, Example, ExampleCollection

from VW.parsers.flatbuffer.Label import *
from VW.parsers.flatbuffer.Namespace import *
from VW.parsers.flatbuffer.Feature import *
from VW.parsers.flatbuffer.ExampleCollection import *
from VW.parsers.flatbuffer.Example import *
from VW.parsers.flatbuffer.SimpleLabel import SimpleLabel
from VW.parsers.flatbuffer.CBLabel import CBLabel
from VW.parsers.flatbuffer.CB_EVAL_Label import CB_EVAL_Label
from VW.parsers.flatbuffer.CCBLabel import CCBLabel
from VW.parsers.flatbuffer.CS_Label import CS_Label
from VW.parsers.flatbuffer.MultiClass import MultiClass
from VW.parsers.flatbuffer.MultiLabel import MultiLabel
from VW.parsers.flatbuffer.no_label import no_label

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-fb','--flatbuffer', help='Path to flatbuffer file', default='test.dat', type=str)
    parser.add_argument('-n', '--number', help="Comma separated strings of example numbers to print", default=None, type=str)

    args = parser.parse_args()

    buffer = open(args.flatbuffer, 'rb').read()

    examples = ExampleCollection.GetRootAsExampleCollection(buffer, 0)

    if examples.ExamplesIsNone():
        print("Nothing in here")

    label_types = {0:None, 1:SimpleLabel, 2:CBLabel, 3:CCBLabel, 4:MultiClass, 5:MultiLabel, 6:CB_EVAL_Label, 7:CS_Label, 8:no_label}

    if args.number is not None:
        egno = args.number.split(',')
        for eg in egno:
            example = examples.Examples(int(eg) - 1)
            label = label_types[example.LabelType()]()
            label.Init(example.Label().Bytes, example.Label().Pos)
            print("{0:.7g} {1}|".format(label.Label(), label.Weight()), end="")
            for i in range(0, example.NamespacesLength()):
                #print("{}".format(example.Namespaces(i).Hash()), end=" ")
                print("{}".format(i), end=" ")
                for j in range(0, example.Namespaces(i).FeaturesLength()):
                    # print("{}:{}".format(example.Namespaces(i).Features(j).Hash(), example.Namespaces(i).Features(j).Value()), end=" ")
                    print("{0}:{1:.7g}".format(j, example.Namespaces(i).Features(j).Value()), end=" ")
                print("|", end="")
            print()

if __name__ == "__main__":
    main()