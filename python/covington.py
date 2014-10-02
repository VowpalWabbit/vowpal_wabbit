import pyvw

# the label for each word is its parent, or -1 for root
my_dataset = [ [("the",      1),   # 0
                ("monster",  2),   # 1
                ("ate",     -1),   # 2
                ("a",        5),   # 3
                ("big",      5),   # 4
                ("sandwich", 2) ]  # 5
                ,
               [("the",      1),   # 0
                ("sandwich", 2),   # 1
                ("is",      -1),   # 2
                ("tasty",    2)]   # 3
                ,
               [("a",        1),   # 0
                ("sandwich", 2),   # 1
                ("ate",     -1),   # 2
                ("itself",   2),   # 3
                ]
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
    
vw = pyvw.vw("--search 2 --quiet --search_task hook --ring_size 1024")
task = vw.init_search_task(CovingtonDepParser)
for p in range(10): # do ten passes over the training data
    task.learn(my_dataset.__iter__)
print 'testing'
print task.predict( [(w,-1) for w in "the monster ate a sandwich".split()] )
print 'should have printed [ 1 2 -1 4 2 ]'
