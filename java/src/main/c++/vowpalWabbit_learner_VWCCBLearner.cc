#include "vowpalWabbit_learner_VWCCBLearner.h"

#include "jni_base_learner.h"
#include "vw/core/vw.h"

jobject decision_scores_prediction(example* vec, JNIEnv* env)
{
  jclass action_score_class = env->FindClass("vowpalWabbit/responses/ActionScore");
  jmethodID action_score_constructor = env->GetMethodID(action_score_class, "<init>", "(IF)V");
  jclass action_scores_class = env->FindClass("vowpalWabbit/responses/ActionScores");
  jmethodID action_scores_constructor =
      env->GetMethodID(action_scores_class, "<init>", "([LvowpalWabbit/responses/ActionScore;)V");
  jclass decision_scores_class = env->FindClass("vowpalWabbit/responses/DecisionScores");
  jmethodID decision_scores_constructor =
      env->GetMethodID(decision_scores_class, "<init>", "([LvowpalWabbit/responses/ActionScores;)V");

  VW::decision_scores_t decision_scores = vec->pred.decision_scores;
  size_t num_slots = decision_scores.size();

  jobjectArray j_action_scores_array = env->NewObjectArray(num_slots, action_scores_class, 0);
  for (uint32_t i = 0; i < num_slots; ++i)
  {
    VW::action_scores a_s = decision_scores[i];
    size_t num_values = a_s.size();
    jobjectArray j_action_score_array = env->NewObjectArray(num_values, action_score_class, 0);
    for (uint32_t j = 0; j < num_values; ++j)
    {
      VW::action_score a = a_s[j];
      jobject j_action_score = env->NewObject(action_score_class, action_score_constructor, a.action, a.score);
      env->SetObjectArrayElement(j_action_score_array, j, j_action_score);
    }
    jobject j_action_scores_object =
        env->NewObject(action_scores_class, action_scores_constructor, j_action_score_array);
    env->SetObjectArrayElement(j_action_scores_array, i, j_action_scores_object);
  }
  return env->NewObject(decision_scores_class, decision_scores_constructor, j_action_scores_array);
}

JNIEXPORT jobject JNICALL Java_vowpalWabbit_learner_VWCCBLearner_predict(
    JNIEnv* env, jobject obj, jstring example_string, jboolean learn, jlong vwPtr)
{
  return base_predict<jobject>(env, example_string, learn, vwPtr, decision_scores_prediction);
}

JNIEXPORT jobject JNICALL Java_vowpalWabbit_learner_VWCCBLearner_predictMultiline(
    JNIEnv* env, jobject obj, jobjectArray example_strings, jboolean learn, jlong vwPtr)
{
  return base_predict<jobject>(env, example_strings, learn, vwPtr, decision_scores_prediction);
}
