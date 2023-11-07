#include "vowpalWabbit_learner_VWActionProbsLearner.h"

#include "jni_base_learner.h"
#include "vw/core/vw.h"

jobject action_probs_prediction(example* vec, JNIEnv* env)
{
  jclass action_prob_class = env->FindClass("vowpalWabbit/responses/ActionProb");
  jmethodID action_prob_constructor = env->GetMethodID(action_prob_class, "<init>", "(IF)V");

  // The action_probs prediction_type_t is just a placeholder identifying when the aciton_scores
  // should be treated as probabilities or scores.  That is why this function references a_s yet returns
  // ActionProbs to the Java side.
  VW::action_scores a_s = vec->pred.a_s;
  size_t num_values = a_s.size();
  jobjectArray j_action_probs = env->NewObjectArray(num_values, action_prob_class, 0);

  jclass action_probs_class = env->FindClass("vowpalWabbit/responses/ActionProbs");
  for (uint32_t i = 0; i < num_values; ++i)
  {
    VW::action_score a = a_s[i];
    jobject j_action_prob = env->NewObject(action_prob_class, action_prob_constructor, a.action, a.score);
    env->SetObjectArrayElement(j_action_probs, i, j_action_prob);
  }
  jmethodID action_probs_constructor =
      env->GetMethodID(action_probs_class, "<init>", "([LvowpalWabbit/responses/ActionProb;)V");
  return env->NewObject(action_probs_class, action_probs_constructor, j_action_probs);
}

JNIEXPORT jobject JNICALL Java_vowpalWabbit_learner_VWActionProbsLearner_predict(
    JNIEnv* env, jobject obj, jstring example_string, jboolean learn, jlong vwPtr)
{
  return base_predict<jobject>(env, example_string, learn, vwPtr, action_probs_prediction);
}

JNIEXPORT jobject JNICALL Java_vowpalWabbit_learner_VWActionProbsLearner_predictMultiline(
    JNIEnv* env, jobject obj, jobjectArray example_strings, jboolean learn, jlong vwPtr)
{
  return base_predict<jobject>(env, example_strings, learn, vwPtr, action_probs_prediction);
}
