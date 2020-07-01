from __future__ import print_function

import sys
from vowpalwabbit import pyvw


class SequenceLabeler(pyvw.SearchTask):
    def __init__(self, vw, sch, num_actions):
        # you must must must initialize the parent class
        # this will automatically store self.sch <- sch, self.vw <- vw
        pyvw.SearchTask.__init__(self, vw, sch, num_actions)

        # you can test program options with sch.po_exists
        # and get their values with sch.po_get -> string and
        # sch.po_get_int -> int
        if sch.po_exists('search'):
            print('found --search')
            print('--search value =', sch.po_get('search'), ', type =', type(sch.po_get('search')))

        # set whatever options you want
        sch.set_options( sch.AUTO_HAMMING_LOSS | sch.AUTO_CONDITION_FEATURES )

    def _run(self, sentence):   # it's called _run to remind you that you shouldn't call it directly!
        output = []
        for n in range(len(sentence)):
            pos,word = sentence[n]
            # use "with...as..." to guarantee that the example is finished properly
            ex = self.vw.example({'w': [word]})
            pred = self.sch.predict(examples=ex, my_tag=n+1, oracle=pos, condition=(n,'p'))
            vw.finish_example([ex]) # must pass the example in as a list because search is a MultiEx reduction
            output.append(pred)
        return output

# wow! your data can be ANY type you want... does NOT have to be VW examples
DET  = 1
NOUN = 2
VERB = 3
ADJ  = 4
my_dataset = [ [(DET , 'the'),
                (NOUN, 'monster'),
                (VERB, 'ate'),
                (DET , 'a'),
                (ADJ , 'big'),
                (NOUN, 'sandwich')],
               [(DET , 'the'),
                (NOUN, 'sandwich'),
                (VERB, 'was'),
                (ADJ , 'tasty')],
               [(NOUN, 'it'),
                (VERB, 'ate'),
                (NOUN, 'it'),
                (ADJ , 'all')] ]


# initialize VW as usual, but use 'hook' as the search_task
vw = pyvw.vw("--search 4 --quiet --search_task hook --ring_size 1024")

# tell VW to construct your search task object
sequenceLabeler = vw.init_search_task(SequenceLabeler)

# train it on the above dataset ten times; the my_dataset.__iter__ feeds into _run above
print('training!', file=sys.stderr)
for i in range(10):
    sequenceLabeler.learn(my_dataset)

# now see the predictions on a test sentence
print('predicting!', file=sys.stderr)
print(sequenceLabeler.predict( [(1,w) for w in "the sandwich ate a monster".split()] ))
print('should have printed: [1, 2, 3, 1, 2]')
