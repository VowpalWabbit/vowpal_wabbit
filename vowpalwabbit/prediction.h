#pragma once

enum class prediction_type_t : int
{
  unset,
  scalar,
  scalars,
  action_scores,
  multiclass,
  multilabels,
  prob,
  decision_scores,

  // These are synonyms for action_scores. They should not be used with this union and should never be the tag.
  multiclassprobs,
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
    TO_STRING_CASE(prediction_type_t::decision_scores)
    TO_STRING_CASE(prediction_type_t::multiclass)
    TO_STRING_CASE(prediction_type_t::multilabels)
    TO_STRING_CASE(prediction_type_t::prob)

    TO_STRING_CASE(prediction_type_t::action_probs)
    TO_STRING_CASE(prediction_type_t::multiclassprobs)
    default:
      return "<unsupported>";
  }
}

struct new_polyprediction
{
 private:
  union {
    float _scalar;
    v_array<float> _scalars;           // a sequence of scalar predictions
    ACTION_SCORE::action_scores _action_scores;  // a sequence of classes with scores.  Also used for probabilities.
    CCB::decision_scores_t _decision_scores;
    uint32_t _multiclass;
    MULTILABEL::labels _multilabels;
    float _prob;  // for --probabilities --csoaa_ldf=mc
  };
  prediction_type_t _tag;

  inline void ensure_is_type(prediction_type_t type) const
  {
    if (_tag != type)
    {
      THROW("Expected type: " << to_string(type) << ", but found: " << to_string(_tag));
    }
  }

  template <typename T>
  void destruct(T& item)
  {
    item.~T();
  }

  // These two functions only differ by parameter
  void copy_from(const new_polyprediction& other)
  {
    reset();
    switch (_tag)
    {
      case (prediction_type_t::unset):
        break;
      case (prediction_type_t::scalar):
        init_as_scalar(other.scalar());
        break;
      case (prediction_type_t::scalars):
        init_as_scalars(other.scalars());
        break;
      case (prediction_type_t::action_scores):
        init_as_action_scores(other.action_scores());
        break;
      case (prediction_type_t::decision_scores):
        init_as_decision_scores(other.decision_scores());
        break;
      case (prediction_type_t::multiclass):
        init_as_multiclass(other.multiclass());
        break;
      case (prediction_type_t::multilabels):
        init_as_multilabels(other.multilabels());
        break;
      case (prediction_type_t::prob):
        init_as_prob(other.prob());
        break;
      default:;
    }
  }

  void move_from(const new_polyprediction&& other)
  {
    reset();
    switch (_tag)
    {
      case (prediction_type_t::unset):
        break;
      case (prediction_type_t::scalar):
        init_as_scalar(other.scalar());
        break;
      case (prediction_type_t::scalars):
        init_as_scalars(other.scalars());
        break;
      case (prediction_type_t::action_scores):
        init_as_action_scores(other.action_scores());
        break;
      case (prediction_type_t::decision_scores):
        init_as_decision_scores(other.decision_scores());
        break;
      case (prediction_type_t::multiclass):
        init_as_multiclass(other.multiclass());
        break;
      case (prediction_type_t::multilabels):
        init_as_multilabels(other.multilabels());
        break;
      case (prediction_type_t::prob):
        init_as_prob(other.prob());
        break;
      default:;
    }
  }

 public:
  new_polyprediction() { _tag = prediction_type_t::unset; // Perhaps we should memset here?
  };
  ~new_polyprediction() { reset(); }

  new_polyprediction(new_polyprediction&& other)
  {
    move_from(std::move(other));
    other.reset();
  }

  new_polyprediction& operator=(new_polyprediction&& other)
  {
    move_from(std::move(other));
    other.reset();
    return *this;
  }

  new_polyprediction(const new_polyprediction& other) {
    copy_from(other);
  }

  new_polyprediction& operator=(const new_polyprediction& other) {
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
        break;
      case (prediction_type_t::scalar):
        destruct(_scalar);
        break;
      case (prediction_type_t::scalars):
        destruct(_scalars);
        break;
      case (prediction_type_t::action_scores):
        destruct(_action_scores);
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
};
