import pylibvw

class vw(pylibvw.vw):
    """The pyvw.vw object is a (trivial) wrapper around the pylibvw.vw
    object; you're probably best off using this directly and ignoring
    the pylibvw.vw structure entirely."""
    
    def __init__(self, argString=""):
        """Initialize the vw object. The (optional) argString is the
        same as the command line arguments you'd use to run vw (eg,"--audit")"""
        pylibvw.vw.__init__(self,argString)
        self.finished = False

    def get_weight(self, index, offset=0):
        """Given an (integer) index (and an optional offset), return
        the weight for that position in the (learned) weight vector."""
        return pylibvw.vw.get_weight(self, index, offset)

    def learn(self, example):
        """Perform an online update; example can either be an example
        object or a string (in which case it is parsed and then
        learned on)."""
        if isinstance(example, str):
            self.learn_string(example)
        else:
            pylibvw.vw.learn(self, example)

    def finish(self):
        """stop VW by calling finish (and, eg, write weights to disk)"""
        if not self.finished:
            pylibvw.vw.finish(self)
            self.finished = True

    def example(self, string=None):
        return example(self, string)

    def __del__(self):
        self.finish()

class namespace_id():
    """The namespace_id class is simply a wrapper to convert between
    hash spaces referred to by character (eg 'x') versus their index
    in a particular example. Mostly used internally, you shouldn't
    really need to touch this."""

    def __init__(self, ex, id):
        """Given an example and an id, construct a namespace_id. The
        id can either be an integer (in which case we take it to be an
        index into ex.indices[]) or a string (in which case we take
        the first character as the namespace id)."""
        if isinstance(id, int):  # you've specified a namespace by index
            if id < 0 or id >= ex.num_namespaces():
                raise Exception('namespace ' + str(id) + ' out of bounds')
            self.id = id
            self.ord_ns = ex.namespace(id)
            self.ns = chr(self.ord_ns)
        elif isinstance(id, str):   # you've specified a namespace by string
            if len(id) == 0:
                id = ' '
            self.id = None  # we don't know and we don't want to do the linear search requered to find it
            self.ns = id[0]
            self.ord_ns = ord(self.ns)
        else:
            raise Exception("ns_to_characterord failed because id type is unknown: " + str(type(id)))

class example_namespace():
    """The example_namespace class is a helper class that allows you
    to extract namespaces from examples and operate at a namespace
    level rather than an example level. Mainly this is done to enable
    indexing like ex['x'][0] to get the 0th feature in namespace 'x'
    in example ex."""
    
    def __init__(self, ex, ns):
        """Construct an example_namespace given an example and a
        target namespace (ns should be a namespace_id)"""
        if not isinstance(ns, namespace_id):
            raise TypeError
        self.ex = ex
        self.ns = ns

    def num_features_in(self):
        """Return the total number of features in this namespace."""
        return self.ex.num_features_in(self.ns)

    def __getitem__(self, i):
        """Get the feature/value pair for the ith feature in this
        namespace."""
        f = self.ex.feature(self.ns, i)
        v = self.ex.feature_weight(self.ns, i)
        return (f, v)

    def iter_features(self):
        """iterate over all feature/value pairs in this namespace."""
        for i in range(self.num_features_in()):
            yield self[i]

    # TODO: def push_feature(self, feature, 

class abstract_label:
    """An abstract class for a VW label."""
    def __init__(self):
        pass

    def from_example(self, ex):
        """grab a label from a given VW example"""
        raise Exception("from_example not yet implemented")

class simple_label(abstract_label):
    def __init__(self, label=0., weight=1., initial=0., prediction=0.):
        abstract_label.__init__(self)
        if isinstance(label, example):
            self.from_example(label)
        else:
            self.label      = label
            self.weight     = weight
            self.initial    = initial
            self.prediction = prediction

    def from_example(self, ex):
        self.label      = ex.get_simplelabel_label()
        self.weight     = ex.get_simplelabel_weight()
        self.initial    = ex.get_simplelabel_initial()
        self.prediction = ex.get_simplelabel_prediction()

    def __str__(self):
        s = str(self.label)
        if self.weight != 1.:
            s += ':' + self.weight
        return s

class multiclass_label(abstract_label):
    def __init__(self, label=1, weight=1., prediction=1):
        abstract_label.__init__(self)
        self.label      = label
        self.weight     = weight
        self.prediction = prediction

    def from_example(self, ex):
        self.label      = ex.get_multiclass_label()
        self.weight     = ex.get_multiclass_weight()
        self.prediction = ex.get_multiclass_prediction()

    def __str__(self):
        s = str(self.label)
        if self.weight != 1.:
            s += ':' + self.weight
        return s

class cost_sensitive_label(abstract_label):
    class wclass:
        def __init__(self, label, cost=0., partial_prediction=0., wap_value=0.):
            self.label = label
            self.cost = cost
            self.partial_prediction = partial_prediction
            self.wap_value = wap_value
    
    def __init__(self, costs=[], prediction=0):
        abstract_label.__init__(self)
        self.costs = costs
        self.prediction = prediction

    def from_example(self, ex):
        self.prediction = ex.get_costsensitive_prediction()
        self.costs = []
        for i in range(ex.get_costsensitive_num_costs):
            wc = wclass(ex.get_costsensitive_class(),
                        ex.get_costsensitive_cost(),
                        ex.get_costsensitive_partial_prediction(),
                        ex.get_costsensitive_wap_value())
            self.costs.append(wc)

    def __str__(self):
        return '[' + ' '.join([str(c.label) + ':' + str(c.cost) for c in self.costs])

class cbandits_label(abstract_label):
    class wclass:
        def __init__(self, label, cost=0., partial_prediction=0., probability=0.):
            self.label = label
            self.cost = cost
            self.partial_prediction = partial_prediction
            self.probability = probability
    
    def __init__(self, costs=[], prediction=0):
        abstract_label.__init__(self)
        self.costs = costs
        self.prediction = prediction

    def from_example(self, ex):
        self.prediction = ex.get_cbandits_prediction()
        self.costs = []
        for i in range(ex.get_cbandits_num_costs):
            wc = wclass(ex.get_cbandits_class(),
                        ex.get_cbandits_cost(),
                        ex.get_cbandits_partial_prediction(),
                        ex.get_cbandits_probability())
            self.costs.append(wc)

    def __str__(self):
        return '[' + ' '.join([str(c.label) + ':' + str(c.cost) for c in self.costs])

class example(pylibvw.example):
    """The example class is a (non-trivial) wrapper around
    pylibvw.example. Most of the wrapping is to make the interface
    easier to use (by making the types safer via namespace_id) and
    also with added python-specific functionality."""
    
    def __init__(self, vw, initString=None):
        """Construct a new example from vw. If initString is None, you
        get an"empty" example which you can construct by hand (see, eg,
        example.push_features). If initString is a string, then this
        string is parsed as it would be from a VW data file into an
        example (and "setup_example" is run)."""
        if initString is None:
            pylibvw.example.__init__(self, vw)
            self.setup_done = False
        else:
            pylibvw.example.__init__(self, vw, initString)
            self.setup_done = True
        self.vw = vw
        self.stride = vw.get_stride()
        self.finished = False

    def __del__(self):
        self.finish()

    def get_ns(self, id):
        """Construct a namespace_id from either an integer or string
        (or, if a namespace_id is fed it, just return it directly)."""
        if isinstance(id, namespace_id):
            return id
        else:
            return namespace_id(self, id)

    def __getitem__(self, id):
        """Get an example_namespace object associated with the given
        namespace id."""
        return example_namespace(self, self.get_ns(id))

    def feature(self, ns, i):
        """Get the i-th hashed feature id in a given namespace (i can
        range from 0 to self.num_features_in(ns)-1)"""
        ns = self.get_ns(ns)  # guaranteed to be a single character
        f = pylibvw.example.feature(self, ns.ord_ns, i)
        if self.setup_done:
            f = (f - self.get_ft_offset()) / self.stride
        return f

    def feature_weight(self, ns, i):
        """Get the value(weight) associated with a given feature id in
        a given namespace (i can range from 0 to
        self.num_features_in(ns)-1)"""
        return pylibvw.example.feature_weight(self, self.get_ns(ns).ord_ns, i)

    def set_label_string(self, string):
        """Give this example a new label, formatted as a string (ala
        the VW data file format)."""
        pylibvw.example.set_label_string(self, self.vw, string)

    def setup_example(self):
        """If this example hasn't already been setup (ie, quadratic
        features constructed, etc.), do so."""
        if self.setup_done:
            raise Exception('trying to setup_example on an example that is already setup')
        self.vw.setup_example(self)
        self.setup_done = True

    def learn(self):
        """Learn on this example (and before learning, automatically
        call setup_example if the example hasn't yet been setup)."""
        if not self.setup_done:
            self.setup_example()
        self.vw.learn(self)

    def sum_feat_sq(self, ns):
        """Return the total sum feature-value squared for a given
        namespace."""
        return pylibvw.example.sum_feat_sq(self, self.get_ns(ns).ord_ns)

    def num_features_in(self, ns):
        """Return the total number of features in a given namespace."""
        return pylibvw.example.num_features_in(self, self.get_ns(ns).ord_ns)

    def get_feature_id(self, ns, feature, ns_hash=None):
        """Return the hashed feature id for a given feature in a given
        namespace. feature can either be an integer (already a feature
        id) or a string, in which case it is hashed. Note that if
        --hash all is on, then get_feature_id(ns,"5") !=
        get_feature_id(ns, 5). If you've already hashed the namespace,
        you can optionally provide that value to avoid re-hashing it."""
        if isinstance(feature, int):
            return feature
        if isinstance(feature, str):
            if ns_hash is None:
                ns_hash = self.vw.hash_space( self.get_ns(ns).ns )
            return self.vw.hash_feature(feature, ns_hash)
        raise Exception("cannot extract feature of type: " + str(type(feature)))


    def push_hashed_feature(self, ns, f, v=1.):
        """Add a hashed feature to a given namespace (fails if setup
        has already run on this example). Fails if setup has run."""
        if self.setup_done: raise Exception("error: modification to example after setup")
        pylibvw.example.push_hashed_feature(self, self.get_ns(ns).ord_ns, f, v)

    def push_feature(self, ns, feature, v=1., ns_hash=None):
        """Add an unhashed feature to a given namespace (fails if
        setup has already run on this example). Fails if setup has
        run."""
        f = self.get_feature_id(ns, feature, ns_hash)
        self.push_hashed_feature(ns, f, v)

    def pop_feature(self, ns):
        """Remove the top feature from a given namespace; returns True
        if a feature was removed, returns False if there were no
        features to pop. Fails if setup has run."""
        if self.setup_done: raise Exception("error: modification to example after setup")
        return pylibvw.example.pop_feature(self, self.get_ns(ns).ord_ns)

    def push_namespace(self, ns):
        """Push a new namespace onto this example. You should only do
        this if you're sure that this example doesn't already have the
        given namespace. Fails if setup has run."""
        if self.setup_done: raise Exception("error: modification to example after setup")
        pylibvw.example.push_namespace(self, self.get_ns(ns).ord_ns)

    def pop_namespace(self):
        """Remove the top namespace from an example; returns True if a
        namespace was removed, or False if there were no namespaces
        left. Fails if setup has run."""
        if self.setup_done: raise Exception("error: modification to example after setup")
        return pylibvw.example.pop_namespace(self)

    def ensure_namespace_exists(self, ns):
        """Check to see if a namespace already exists. If it does, do
        nothing. If it doesn't, add it. Fails if setup has run."""
        if self.setup_done: raise Exception("error: modification to example after setup")
        return pylibvw.example.ensure_namespace_exists(self, self.get_ns(ns).ord_ns)

    def push_features(self, ns, featureList):
        """Push a list of features to a given namespace. Each feature
        in the list can either be an integer (already hashed) or a
        string (to be hashed) and may be paired with a value or not
        (if not, the value is assumed to be 1.0).

        Examples:
           ex.push_features('x', ['a', 'b'])
           ex.push_features('y', [('c', 1.), 'd'])

           space_hash = vw.hash_space( 'x' )
           feat_hash  = vw.hash_feature( 'a', space_hash )
           ex.push_features('x', [feat_hash])    # note: 'x' should match the space_hash!

        Fails if setup has run."""
        ns = self.get_ns(ns)
        self.ensure_namespace_exists(ns)
        ns_hash = self.vw.hash_space(ns.ns)
        
        for feature in featureList:
            if isinstance(feature, int) or isinstance(feature, str):
                f = feature
                v = 1.
            elif isinstance(feature, tuple) and len(feature) == 2:
                f = feature[0]
                v = feature[1]
            else:
                raise Exception('malformed feature to push of type: ' + str(type(feature)))

            self.push_feature(ns, f, v, ns_hash)

    def finish(self):
        """Tell VW that you're done with this example and it can
        recycle it for later use."""
        if not self.finished:
            self.vw.finish_example(self)
            self.finished = True

    def iter_features(self):
        """Iterate over all feature/value pairs in this example (all
        namespace included)."""
        for ns_id in range( self.num_namespaces() ):  # iterate over every namespace
            ns = self.get_ns(ns_id)
            for i in range(self.num_features_in(ns)):
                f = self.feature(ns, i)
                v = self.feature_weight(ns, i)
                yield f,v

    def get_label(self, label_class=simple_label):
        """Given a known label class (default is simple_label), get
        the corresponding label structure for this example."""
        return label_class(self)

#help(example)
