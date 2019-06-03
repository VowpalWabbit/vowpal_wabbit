from __future__ import print_function

from vowpalwabbit import pyvw

# the dataset is triples of E, A, F where A[i] = list of words E_i
# aligned to, or [] for null-aligned
my_dataset = [
    ( "the blue house".split(),
      ([0], [2], [1]),
      "la maison bleue".split() ),
    ( "the house".split(),
      ([0], [1]),
      "la maison".split() ),
    ( "the flower".split(),
      ([0], [1]),
      "la fleur".split() )
    ]

my_dataset2 = [
    ( "mary did not slap    the green witch".split(),
      ([0], [], [1],[2,3,4],[6],[8],  [7]),
      "maria no dio una bofetada a la bruja verde".split() ) ]
      #  0   1   2   3     4     5  6   7     8


def alignmentError(true, sys):
    t = set(true)
    s = set(sys)
    if len(t | s) == 0: return 0.
    return 1. - float(len(t & s)) / float(len(t | s))


class WordAligner(pyvw.SearchTask):
    def __init__(self, vw, sch, num_actions):
        pyvw.SearchTask.__init__(self, vw, sch, num_actions)
        sch.set_options( sch.AUTO_HAMMING_LOSS | sch.IS_LDF | sch.AUTO_CONDITION_FEATURES )

    def makeExample(self, E, F, i, j0, l):
        f  = 'Null' if j0 is None else [ F[j0+k] for k in range(l+1) ]
        ex = self.vw.example( { 'e': E[i],
                                'f': f,
                                'p': '_'.join(f),
                                'l': str(l),
                                'o': [str(i-j0), str(i-j0-l)] if j0 is not None else [] },
                              labelType = self.vw.lCostSensitive )
        lab = 'Null' if j0 is None else str(j0+l)
        ex.set_label_string(lab + ':0')
        return ex

    def _run(self, alignedSentence):
        E,A,F = alignedSentence

        # for each E word, we pick a F span
        covered = {}  # which F words have been covered so far?
        output  = []

        for i in range(len(E)):
            examples = []  # contains vw examples
            spans    = []  # contains triples (alignment error, index in examples, [range])

            # empty span:
            examples.append( self.makeExample(E, F, i, None, None) )
            spans.append( (alignmentError(A[i], []), 0, []) )

            # non-empty spans
            for j0 in range(len(F)):
                for l in range(3):   # max phrase length of 3
                    if j0+l >= len(F):
                        break
                    if j0+l in covered:
                        break

                    id = len(examples)
                    examples.append( self.makeExample(E, F, i, j0, l) )
                    spans.append( (alignmentError(A[i], list(range(j0,j0+l+1))), id, list(range(j0,j0+l+1))) )

            sortedSpans = []
            for s in spans: sortedSpans.append(s)
            sortedSpans.sort()
            oracle = []
            for id in range(len(sortedSpans)):
                if sortedSpans[id][0] > sortedSpans[0][0]: break
                oracle.append( sortedSpans[id][1] )

            pred = self.sch.predict(examples  = examples,
                                    my_tag    = i+1,
                                    oracle    = oracle,
                                    condition = [ (i, 'p'), (i-1, 'q') ] )

            self.vw.finish_example(examples)

            output.append( spans[pred][2] )
            for j in spans[pred][2]:
                covered[j] = True

        return output


print('training LDF')
vw = pyvw.vw("--search 0 --csoaa_ldf m --search_task hook --ring_size 1024 --quiet -q ef -q ep")
task = vw.init_search_task(WordAligner)
for p in range(10):
    task.learn(my_dataset)
print('====== test ======')
print(task.predict( ("the blue flower".split(), ([],[],[]), "la fleur bleue".split()) ))
print('should have printed [[0], [2], [1]]')
