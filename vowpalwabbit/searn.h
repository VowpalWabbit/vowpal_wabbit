#ifndef SEARN_H
#define SEARN_H

#include <stdio.h>
#include "parse_args.h"
#include "oaa.h"
#include "parse_primitives.h"

#define clog_print_audit_features(ec,reg) { print_audit_features(reg, ec); }


#define MAX_ACTION_ID   10000

typedef void*  state;
typedef size_t action;     // actions are ONE-based
typedef uint32_t* history;

namespace SearnUtil
{
  struct history_info {
    size_t length;          // was history_length
    bool   bigrams;         // was sequence_bigrams
    size_t features;        // was sequence_features
    bool   bigram_features; // was sequence_bigram_features
  };
  void default_info(history_info*);


  void* calloc_or_die(size_t, size_t);
  void free_it(void*);

  int  random_policy(long int, float, bool, int, bool);

  void add_policy_offset(vw&, example*, size_t, size_t, size_t);
  void remove_policy_offset(vw&, example*, size_t, size_t, size_t);

  void add_history_to_example(vw&, history_info*, example*, history);
  void remove_history_from_example(vw&, history_info *, example*);
}

namespace Searn
{
  struct search_task {
    /************************************************************************
     ******************* REQUIRED FUNCTIONS FOR ALL TASKS *******************
     ***********************************************************************/

    // final(state) tells me whether state is a final(=goal) state or note
    bool   (*final)(state);

    // loss(state) on FINAL states gives the cumulative loss for that state.
    // for non-final states, you can either (a) return MAX_FLT or, if
    // possible, (b) return a partial loss up to this point
    float  (*loss)(state);

    // step(state, action) takes a state and an action and advances that
    // state *in place* according to the action
    void   (*step)(state, action);

    // oracle(state) takes a state and tells me (some) oracle action that
    // I can take from that state.
    action (*oracle)(state);

    // copy(src) copies the source state into the destination.  YOU
    // are responsible for allocating the destination.  you can deallocate
    // it in the finish function below
    // note: if you provide hash or equivalent, we assume that a copied
    // state will be equivalent to and have the same hash value as the
    // original
    state  (*copy)(state);

    // finish(state) should de-allocate any memory associated with state.
    // we will call finish precisely once per state created either through
    // copy or through one of the start_state functions
    void   (*finish)(state);

    // you must provide a label parser for reading data
    label_parser searn_label_parser;

    /************************************************************************
     ******************* CHOOSE ONE OF THE FOLLOWING TWO ********************
     ***********************************************************************/

    // if you expect to read one example per line in the data file,
    // you should provide an implementation of start_state(ec, state).
    // this should take the example read from the file and generate a
    // start state corresponding to that example.  you will need to allocate
    // the memory for this state (will be freed in finish).
    void   (*start_state)(example*, state*);

    // if you expect to read multiple lines per example (with blank
    // lines separating them), then implement this function.  you will
    // be given an array of examples and the length of that array (=the
    // number of lines) and you must return a state (see start_state).
    void   (*start_state_multiline)(example**, size_t, state*);

    /************************************************************************
     ******************* CHOOSE ONE OF THE FOLLOWING TWO ********************
     ***********************************************************************/

    // if your application does not require label dependent features
    // (aka the same set of actions are available at each time step
    // and have the same semantics) then implement cs_example.  this
    // takes a state, a reference to an example, and a flag indicating
    // whether you are CREATING (true) or DESTROYING (false) the
    // example.  if creating, then you should fill in the example
    // reference with an example.  if you need to allocate memory to
    // do this, you will have the opportunity to free it when
    // cs_example is called with the flag set to false.  similarly,
    // if you take the example from your example set (from
    // start_state), then you should be sure to un-manipulate it in
    // the destroy call.
    void   (*cs_example)(vw&, state, example*&, bool);

    // if you need label dependent features, then you will need to
    // construct (and destroy) a separate example for each action.
    // important: an example is constructed for EVERY action for a
    // given state before any of them are destroyed, so don't reuse
    // the same memory.  note: if you are using cs_ldf_example, you
    // MUST provide the allowed() function, see below.
    void   (*cs_ldf_example)(vw&, state, action, example*&, bool);

    /************************************************************************
     ********************* (MOSTLY) OPTIONAL FUNCTIONS **********************
     ***********************************************************************/

    // your task might need to initialize some memory at startup or
    // parse command line arguments: do that in initialize
    bool   (*initialize)(std::vector<std::string>&opts, po::variables_map& vm);

    // your task might need to free some memory at the end of running:
    // do that in finalize
    void   (*finalize)();

    // if you wish to _enable_ (but not force!) hypothesis recombination
    // during rollouts, you'll need to provide an equivalence function
    // between two states.  the semantics is: two states are equivalent
    // if they are indistinguishable from the perspective of future
    // predictions.  for instance, in a bigram model, once two states
    // have the same most-recent-prediction, they become equivalent.
    // if you provide this function, you MUST provide hash (see below)
    bool   (*equivalent)(state, state);

    // hash should take a state and return a hash value.  this is used
    // in two ways.  FIRST, when we roll-out policies and need to choose
    // a random policy (in searn), we will use the hash of the state
    // as a seed to the PRNG.  if you don't give us a hash, we'll use
    // the position in rollout, but this is not ideal if different steps
    // can consume different amounts of the input.  SECOND, it is used
    // to do efficient hypothesis recombination, in which case you must
    // obey the semantics that equivalent(a,b) ==> hash(a)==hash(b) and,
    // preferably, hash(a)!=hash(b) means that it's very likely that
    // a and b are not equivalent.
    size_t (*hash)(state);

    // * in the case of LDF: allowed is a required function.  we will
    // try actions 1, 2, ... and so on provided allowed(state,action)
    // returns true.  once it returns false, we'll stop (and will not
    // use the action id on which false was returned).
    // * in the case of non-LDF: allowed is an optional function.  we
    // will always try all actions 1, 2, ... max_action and you may
    // return any of them as invalid
    bool   (*allowed)(state, action);

    // to_string should return a string representation of some data.
    // we'll give you a state and a bool specifying whether we want
    // the TRUE output or the PREDICTED output.  in the case that we
    // ask for the predicted output, we'll give you a sequence of
    // actions corresponding to that prediction.  note: we make
    // no guarantee that the state we give you is start or end or
    // anything...
    std::string (*to_string)(state, bool, std::vector<action>);
  };


  void parse_flags(vw&all, std::vector<std::string>&, po::variables_map& vm, void (*base_l)(vw&,example*), void (*base_f)(vw&));
  void drive(void*);
}

#endif
