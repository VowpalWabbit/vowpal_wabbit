#pragma once

/*
When a new prediction type needs to be added the following actions must be taken:
- PREDICTION_TYPE is the type that will be used
- PREDICTION_NAME is the name to identify this label type
Steps:
  1. Add a new variant to prediction_type_t called PREDICTION_NAME
  2. Add the corresponding row to to_string:
    TO_STRING_CASE(prediction_type_t::PREDICTION_NAME)
  3. Add the new type to the union:
    PREDICTION_TYPE _PREDICTION_NAME;
  3. Add the corresponding row to polyprediction::copy_from
    case (prediction_type_t::PREDICTION_NAME):
      init_as_PREDICTION_NAME(std::move(other._PREDICTION_NAME));
      break;
  4. Add the corresponding row to polyprediction::move_from
    case (prediction_type_t::PREDICTION_NAME):
      init_as_PREDICTION_NAME(std::move(other._PREDICTION_NAME));
      break;
  5. Add the corresponding row to polyprediction::reset
    case (prediction_type_t::PREDICTION_NAME):
        destruct(_PREDICTION_NAME);
        break;
  6. Add another three methods that correspond to the new type according to this template
    template <typename... Args>
    PREDICTION_TYPE& init_as_PREDICTION_NAME(Args&&... args)
    {
      ensure_is_type(prediction_type_t::unset);
      new (&_PREDICTION_NAME) PREDICTION_TYPE(std::forward<Args>(args)...);
      _tag = prediction_type_t::PREDICTION_NAME;
      return _PREDICTION_NAME;
    }

    const PREDICTION_TYPE& PREDICTION_NAME() const
    {
      ensure_is_type(prediction_type_t::PREDICTION_NAME);
      return _PREDICTION_NAME;
    }

    PREDICTION_TYPE& PREDICTION_NAME()
    {
      ensure_is_type(prediction_type_t::PREDICTION_NAME);
      return _PREDICTION_NAME;
    }
*/

enum class prediction_type_t : int
{
  unset,
  scalar,
  scalars,
  action_scores,
  multiclassprobs,
  multiclass,
  multilabels,
  prob,
  decision_scores,
  action_probs,
};

#define TO_STRING_CASE(enum_type) \
  case enum_type:                 \
    return #enum_type;

inline const char* to_string(prediction_type_t prediction_type)
{
  switch (prediction_type)
  {
    TO_STRING_CASE(prediction_type_t::unset)
    TO_STRING_CASE(prediction_type_t::scalar)
    TO_STRING_CASE(prediction_type_t::scalars)
    TO_STRING_CASE(prediction_type_t::action_scores)
    TO_STRING_CASE(prediction_type_t::action_probs)
    TO_STRING_CASE(prediction_type_t::decision_scores)
    TO_STRING_CASE(prediction_type_t::multiclass)
    TO_STRING_CASE(prediction_type_t::multilabels)
    TO_STRING_CASE(prediction_type_t::prob)
    TO_STRING_CASE(prediction_type_t::multiclassprobs)
    default:
      return "<unsupported>";
  }
}

struct polyprediction
{
 private:
  union {
    float _scalar;
    v_array<float> _scalars;           // a sequence of scalar predictions
    ACTION_SCORE::action_scores _action_scores;  // a sequence of classes with scores.
    ACTION_SCORE::action_scores _action_probs;  // a sequence of classes with probs.
    CCB::decision_scores_t _decision_scores;
    uint32_t _multiclass;
    MULTILABEL::labels _multilabels;
    float _prob;  // for --probabilities --csoaa_ldf=mc
    v_array<float> _multiclassprobs;

  };
  prediction_type_t _tag;

  inline void ensure_is_type(prediction_type_t type) const
  {
#ifndef NDEBUG
    if (_tag != type)
    {
      THROW("Expected type: " << to_string(type) << ", but found: " << to_string(_tag));
    }
#else
    _UNUSED(type);
#endif
  }

  template <typename T>
  void destruct(T& item)
  {
    item.~T();
  }

  // These two functions only differ by parameter
  void copy_from(const polyprediction& other)
  {
    switch (other._tag)
    {
      case (prediction_type_t::unset):
        break;
      case (prediction_type_t::scalar):
        init_as_scalar(other._scalar);
        break;
      case (prediction_type_t::scalars):
        init_as_scalars(other._scalars);
        break;
      case (prediction_type_t::action_scores):
        init_as_action_scores(other._action_scores);
        break;
      case (prediction_type_t::action_probs):
        init_as_action_probs(other._action_probs);
        break;
      case (prediction_type_t::decision_scores):
        init_as_decision_scores(other._decision_scores);
        break;
      case (prediction_type_t::multiclass):
        init_as_multiclass(other._multiclass);
        break;
      case (prediction_type_t::multilabels):
        init_as_multilabels(other._multilabels);
        break;
      case (prediction_type_t::prob):
        init_as_prob(other._prob);
        break;
      case (prediction_type_t::multiclassprobs):
        init_as_multiclassprobs(other._multiclassprobs);
        break;
      default:;
    }
  }

  void move_from(polyprediction&& other)
  {
    switch (other._tag)
    {
      case (prediction_type_t::unset):
        break;
      case (prediction_type_t::scalar):
        init_as_scalar(std::move(other._scalar));
        break;
      case (prediction_type_t::scalars):
        init_as_scalars(std::move(other._scalars));
        break;
      case (prediction_type_t::action_scores):
        init_as_action_scores(std::move(other._action_scores));
        break;
      case (prediction_type_t::action_probs):
        init_as_action_probs(std::move(other._action_probs));
        break;
      case (prediction_type_t::decision_scores):
        init_as_decision_scores(std::move(other._decision_scores));
        break;
      case (prediction_type_t::multiclass):
        init_as_multiclass(std::move(other._multiclass));
        break;
      case (prediction_type_t::multilabels):
        init_as_multilabels(std::move(other._multilabels));
        break;
      case (prediction_type_t::prob):
        init_as_prob(std::move(other._prob));
        break;
      case (prediction_type_t::multiclassprobs):
        init_as_multiclassprobs(std::move(other._multiclassprobs));
        break;
      default:;
    }
  }

 public:
  polyprediction() { _tag = prediction_type_t::unset; // Perhaps we should memset here?
  };
  ~polyprediction() { reset(); }

  polyprediction(polyprediction&& other)
  {
    _tag = prediction_type_t::unset;
    move_from(std::move(other));
    other.reset();
  }

  polyprediction& operator=(polyprediction&& other)
  {
    reset();
    move_from(std::move(other));
    other.reset();
    return *this;
  }

  polyprediction(const polyprediction& other) {
    _tag = prediction_type_t::unset;
    copy_from(other);
  }

  polyprediction& operator=(const polyprediction& other) {
    reset();
    copy_from(other);
    return *this;
  }

  prediction_type_t get_type() const { return _tag; }

  void reset()
  {
    switch (_tag)
    {
      case (prediction_type_t::unset):
        // Nothing to do! Whatever was in here has already been destroyed.
        return;
      case (prediction_type_t::scalar):
        destruct(_scalar);
        break;
      case (prediction_type_t::scalars):
        destruct(_scalars);
        break;
      case (prediction_type_t::action_scores):
        destruct(_action_scores);
        break;
      case (prediction_type_t::action_probs):
        destruct(_action_probs);
        break;
      case (prediction_type_t::decision_scores):
        destruct(_decision_scores);
        break;
      case (prediction_type_t::multiclass):
        destruct(_multiclass);
        break;
      case (prediction_type_t::multilabels):
        destruct(_multilabels);
        break;
      case (prediction_type_t::prob):
        destruct(_prob);
        break;
      case (prediction_type_t::multiclassprobs):
        destruct(_multiclassprobs);
        break;
      default:;
    }

    _tag = prediction_type_t::unset;
  }

  template <typename... Args>
  float& init_as_scalar(Args&&... args)
  {
    ensure_is_type(prediction_type_t::unset);
    new (&_scalar) float(std::forward<Args>(args)...);
    _tag = prediction_type_t::scalar;
    return _scalar;
  }

  const float& scalar() const
  {
    ensure_is_type(prediction_type_t::scalar);
    return _scalar;
  }

  float& scalar()
  {
    ensure_is_type(prediction_type_t::scalar);
    return _scalar;
  }

  template <typename... Args>
  v_array<float>& init_as_scalars(Args&&... args)
  {
    ensure_is_type(prediction_type_t::unset);
    new (&_scalars) v_array<float>(std::forward<Args>(args)...);
    _tag = prediction_type_t::scalars;
    return _scalars;
  }

  const v_array<float>& scalars() const
  {
    ensure_is_type(prediction_type_t::scalars);
    return _scalars;
  }

  v_array<float>& scalars()
  {
    ensure_is_type(prediction_type_t::scalars);
    return _scalars;
  }

  template <typename... Args>
  ACTION_SCORE::action_scores& init_as_action_scores(Args&&... args)
  {
    ensure_is_type(prediction_type_t::unset);
    new (&_action_scores) ACTION_SCORE::action_scores(std::forward<Args>(args)...);
    _tag = prediction_type_t::action_scores;
    return _action_scores;
  }

  const ACTION_SCORE::action_scores& action_scores() const
  {
    ensure_is_type(prediction_type_t::action_scores);
    return _action_scores;
  }

  ACTION_SCORE::action_scores& action_scores()
  {
    ensure_is_type(prediction_type_t::action_scores);
    return _action_scores;
  }
  
  template <typename... Args>
  ACTION_SCORE::action_scores& init_as_action_probs(Args&&... args)
  {
    ensure_is_type(prediction_type_t::unset);
    new (&_action_probs) ACTION_SCORE::action_scores(std::forward<Args>(args)...);
    _tag = prediction_type_t::action_probs;
    return _action_probs;
  }

  const ACTION_SCORE::action_scores& action_probs() const
  {
    ensure_is_type(prediction_type_t::action_probs);
    return _action_probs;
  }

  ACTION_SCORE::action_scores& action_probs()
  {
    ensure_is_type(prediction_type_t::action_probs);
    return _action_probs;
  }

  template <typename... Args>
  CCB::decision_scores_t& init_as_decision_scores(Args&&... args)
  {
    ensure_is_type(prediction_type_t::unset);
    new (&_decision_scores) CCB::decision_scores_t(std::forward<Args>(args)...);
    _tag = prediction_type_t::decision_scores;
    return _decision_scores;
  }

  const CCB::decision_scores_t& decision_scores() const
  {
    ensure_is_type(prediction_type_t::decision_scores);
    return _decision_scores;
  }

  CCB::decision_scores_t& decision_scores()
  {
    ensure_is_type(prediction_type_t::decision_scores);
    return _decision_scores;
  }

  template <typename... Args>
  uint32_t& init_as_multiclass(Args&&... args)
  {
    ensure_is_type(prediction_type_t::unset);
    new (&_multiclass) uint32_t(std::forward<Args>(args)...);
    _tag = prediction_type_t::multiclass;
    return _multiclass;
  }

  const uint32_t& multiclass() const
  {
    ensure_is_type(prediction_type_t::multiclass);
    return _multiclass;
  }

  uint32_t& multiclass()
  {
    ensure_is_type(prediction_type_t::multiclass);
    return _multiclass;
  }

  template <typename... Args>
  MULTILABEL::labels& init_as_multilabels(Args&&... args)
  {
    ensure_is_type(prediction_type_t::unset);
    new (&_multilabels) MULTILABEL::labels(std::forward<Args>(args)...);
    _tag = prediction_type_t::multilabels;
    return _multilabels;
  }

  const MULTILABEL::labels& multilabels() const
  {
    ensure_is_type(prediction_type_t::multilabels);
    return _multilabels;
  }

  MULTILABEL::labels& multilabels()
  {
    ensure_is_type(prediction_type_t::multilabels);
    return _multilabels;
  }

  template <typename... Args>
  float& init_as_prob(Args&&... args)
  {
    ensure_is_type(prediction_type_t::unset);
    new (&_prob) float(std::forward<Args>(args)...);
    _tag = prediction_type_t::prob;
    return _prob;
  }

  const float& prob() const
  {
    ensure_is_type(prediction_type_t::prob);
    return _prob;
  }

  float& prob()
  {
    ensure_is_type(prediction_type_t::prob);
    return _prob;
  }
  
  template <typename... Args>
  v_array<float>& init_as_multiclassprobs(Args&&... args)
  {
    ensure_is_type(prediction_type_t::unset);
    new (&_multiclassprobs) v_array<float>(std::forward<Args>(args)...);
    _tag = prediction_type_t::multiclassprobs;
    return _multiclassprobs;
  }

  const v_array<float>& multiclassprobs() const
  {
    ensure_is_type(prediction_type_t::multiclassprobs);
    return _multiclassprobs;
  }

  v_array<float>& multiclassprobs()
  {
    ensure_is_type(prediction_type_t::multiclassprobs);
    return _multiclassprobs;
  }

  // TODO: make this more generic through traits and type comparisons.
  void reinterpret(prediction_type_t type)
  {
    // Currently the only valid reinterpret is between action scores and probs, or itself.
    if((type == prediction_type_t::action_probs && _tag == prediction_type_t::action_scores)
      || (type == prediction_type_t::action_scores && _tag == prediction_type_t::action_probs)
      || type == _tag) 
    {
      _tag = type;
    }
    else
    {
      THROW("Illegal reinterpret. Tried to reinterpret as " << to_string(type) << ", but contains: " << to_string(_tag));
    }
  }
};
