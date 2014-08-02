import sys
import pyvw

class SequenceLabeler(pyvw.SearchTask):
    def __init__(self, vw, srn, num_actions):
        # you must must must initialize the parent class
        # this will automatically store self.srn <- srn, self.vw <- vw
        pyvw.SearchTask.__init__(self, vw, srn, num_actions)

        # you can test program options with srn.po_exists
        # and get their values with srn.po_get -> string and
        # srn.po_get_int -> int
        if srn.po_exists('search'):
            print 'found --search'
            print '--search value =', srn.po_get('search'), ', type =', type(srn.po_get('search'))
        
        # set whatever options you want
        srn.set_options( srn.AUTO_HAMMING_LOSS | srn.AUTO_HISTORY )

    def _run(self, sentence):   # it's called _run to remind you that you shouldn't call it directly!
        output = []
        for tag,word in sentence:
            # use "with...as..." to guarantee that the example is finished properly
            with self.vw.example({'w': [word]}) as ex:
                pred = self.srn.predict(ex, tag)
                output.append(pred)
        return output
    
# wow! your data can be ANY type you want... does NOT have to be VW examples
my_dataset = [ [(1, 'the'),
                (2, 'monster'),
                (3, 'ate'),
                (1, 'a'),
                (4, 'big'),
                (2, 'sandwich')],
               [(1, 'the'),
                (2, 'sandwich'),
                (3, 'was'),
                (4, 'tasty')],
               [(2, 'it'),
                (3, 'ate'),
                (2, 'it'),
                (4, 'all')] ]


# initialize VW as usual, but use 'python_hook' as the search_task
vw = pyvw.vw("--search 4 --quiet --search_task python_hook --search_no_snapshot --ring_size 1024")

# tell VW to construct your search task object
sequenceLabeler = vw.init_search_task(SequenceLabeler)

# train it on the above dataset ten times
print >>sys.stderr, 'training!'
for curPass in range(10):
    sequenceLabeler.learn(my_dataset.__iter__)

# now see the predictions on a test sentence
print >>sys.stderr, 'predicting!'
print sequenceLabeler.predict( [(0,w) for w in "the sandwich ate a monster".split()] )

