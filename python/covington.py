import pyvw

# the label for each word is its parent, or -1 for root
my_dataset = [ [("the",      1),   # 0
                ("monster",  2),   # 1
                ("ate",     -1),   # 2
                ("a",        5),   # 3
                ("big",      5),   # 4
                ("sandwich", 2)]  # 5
                ,
               [("the",      1),   # 0
                ("sandwich", 2),   # 1
                ("is",      -1),   # 2
                ("tasty",    2)]   # 3
                ,
               [("a",        1),   # 0
                ("sandwich", 2),   # 1
                ("ate",     -1),   # 2
                ("itself",   2)]   # 3
                ]

class CovingtonDepParser(pyvw.SearchTask):
    def __init__(self, vw, sch, num_actions):
        pyvw.SearchTask.__init__(self, vw, sch, num_actions)
        sch.set_options( sch.AUTO_HAMMING_LOSS | sch.AUTO_CONDITION_FEATURES )

    def _run(self, sentence):
        N = len(sentence)
        # initialize our output so everything is a root
        output = [-1 for i in range(N)]
        for n in range(N):
            wordN,parN = sentence[n]
            for m in range(-1,N):
                if m == n: continue
                wordM = sentence[m][0] if m > 0 else "*root*"
                # ask the question: is m the parent of n?
                isParent = 2 if m == parN else 1

                # construct an example
                dir = 'l' if m < n else 'r'
                with self.vw.example({'a': [wordN, dir + '_' + wordN], 'b': [wordM, dir + '_' + wordN], 'p': [wordN + '_' + wordM, dir + '_' + wordN + '_' + wordM],
                                      'd': [ str(m-n <= d) + '<=' + str(d) for d in [-8, -4, -2, -1, 1, 2, 4, 8] ] +
                                           [ str(m-n >= d) + '>=' + str(d) for d in [-8, -4, -2, -1, 1, 2, 4, 8] ] }) as ex:
                    pred = self.sch.predict(examples  = ex,
                                            my_tag    = (m+1)*N + n + 1,
                                            oracle    = isParent,
                                            condition = [ (max(0, (m  )*N + n + 1), 'p'),
                                                          (max(0, (m+1)*N + n    ), 'q') ])
                    if pred == 2:
                        output[n] = m
                        break
        return output

class CovingtonDepParserLDF(pyvw.SearchTask):
    def __init__(self, vw, sch, num_actions):
        pyvw.SearchTask.__init__(self, vw, sch, num_actions)
        sch.set_options( sch.AUTO_HAMMING_LOSS | sch.IS_LDF | sch.AUTO_CONDITION_FEATURES )

    def makeExample(self, sentence, n, m):
        wordN = sentence[n][0]
        wordM = sentence[m][0] if m >= 0 else '*ROOT*'
        dir   = 'l' if m < n else 'r'
        ex = self.vw.example( { 'a': [wordN, dir + '_' + wordN],
                                'b': [wordM, dir + '_' + wordN],
                                'p': [wordN + '_' + wordM, dir + '_' + wordN + '_' + wordM],
                                'd': [ str(m-n <= d) + '<=' + str(d) for d in [-8, -4, -2, -1, 1, 2, 4, 8] ] +
                                     [ str(m-n >= d) + '>=' + str(d) for d in [-8, -4, -2, -1, 1, 2, 4, 8] ] },
                              labelType=self.vw.lCostSensitive)
        # the label string is 100+n-m.. the 100 is to make sure
        # negative values of (n-m) are still >=1; the reason we use
        # this label rather than just, eg, m (which is what we're
        # trying to predict) is that we expect the conditioning
        # feature(s) automatically created from positional _deltas_
        # are more useful than absolute positions.
        ex.set_label_string(str(100 + n - m) + ":0")
        return ex
            
    def _run(self, sentence):
        N = len(sentence)
        # initialize our output so everything is a root
        output = [-1 for i in range(N)]
        for n in range(N):
            # make LDF examples
            examples = [ self.makeExample(sentence,n,m) for m in range(-1,N) if n != m ]

            # truth
            parN = sentence[n][1]
            oracle = parN+1 if parN < n else parN   # have to -1 because we excluded n==m from list

            # make a prediction
            pred = self.sch.predict(examples  = examples,
                                    my_tag    = n+1,
                                    oracle    = oracle,
                                    condition = [ (n-1, 'p'), (n-2, 'q') ] )

            output[n] = pred-1 if pred < n else pred # have to +1 because n==m excluded

            for ex in examples: ex.finish()  # clean up
            
        return output

# TODO: if they make sure search=0 <==> ldf <==> csoaa_ldf

vw = pyvw.vw("--search 0 --csoaa_ldf m --search_task hook --ring_size 1024 -q ab")
task = vw.init_search_task(CovingtonDepParserLDF)
for p in range(2): # do two passes over the training data
    task.learn(my_dataset.__iter__)
print 'testing'
print task.predict( [(w,-1) for w in "the monster ate a sandwich".split()] )
print 'should have printed [ 1 2 -1 4 2 ]'
