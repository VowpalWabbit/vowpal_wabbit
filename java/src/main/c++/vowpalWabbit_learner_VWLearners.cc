#include "vowpalWabbit_learner_VWLearners.h"

#include "jni_base_learner.h"
#include "vw/core/vw.h"

#define RETURN_TYPE "vowpalWabbit/learner/VWLearners$VWReturnType"
#define RETURN_TYPE_INSTANCE "L" RETURN_TYPE ";"

JNIEXPORT jlong JNICALL Java_vowpalWabbit_learner_VWLearners_initialize(JNIEnv* env, jclass obj, jstring command)
{
  jlong vwPtr = 0;
  try
  {
    VW::workspace* vwInstance = VW::initialize(env->GetStringUTFChars(command, NULL));
    vwPtr = (jlong)vwInstance;
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
  }
  return vwPtr;
}

JNIEXPORT void JNICALL Java_vowpalWabbit_learner_VWLearners_performRemainingPasses(JNIEnv* env, jclass obj, jlong vwPtr)
{
  try
  {
    VW::workspace* vwInstance = (VW::workspace*)vwPtr;
    if (vwInstance->numpasses > 1)
    {
      vwInstance->do_reset_source = true;
      VW::start_parser(*vwInstance);
      VW::LEARNER::generic_driver(*vwInstance);
      VW::end_parser(*vwInstance);
    }
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT void JNICALL Java_vowpalWabbit_learner_VWLearners_closeInstance(JNIEnv* env, jclass obj, jlong vwPtr)
{
  try
  {
    VW::workspace* vwInstance = (VW::workspace*)vwPtr;
    VW::finish(*vwInstance);
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT void JNICALL Java_vowpalWabbit_learner_VWLearners_saveModel(
    JNIEnv* env, jclass obj, jlong vwPtr, jstring filename)
{
  try
  {
    const char* utf_string = env->GetStringUTFChars(filename, NULL);
    std::string filenameCpp(utf_string);
    env->ReleaseStringUTFChars(filename, utf_string);
    env->DeleteLocalRef(filename);
    VW::save_predictor(*(VW::workspace*)vwPtr, filenameCpp);
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT jobject JNICALL Java_vowpalWabbit_learner_VWLearners_getReturnType(JNIEnv* env, jclass obj, jlong vwPtr)
{
  jclass clVWReturnType = env->FindClass(RETURN_TYPE);
  jfieldID field;
  VW::workspace* vwInstance = (VW::workspace*)vwPtr;
  switch (vwInstance->l->get_output_prediction_type())
  {
    case VW::prediction_type_t::action_probs:
      field = env->GetStaticFieldID(clVWReturnType, "ActionProbs", RETURN_TYPE_INSTANCE);
      break;
    case VW::prediction_type_t::action_scores:
      field = env->GetStaticFieldID(clVWReturnType, "ActionScores", RETURN_TYPE_INSTANCE);
      break;
    case VW::prediction_type_t::multiclass:
      field = env->GetStaticFieldID(clVWReturnType, "Multiclass", RETURN_TYPE_INSTANCE);
      break;
    case VW::prediction_type_t::multilabels:
      field = env->GetStaticFieldID(clVWReturnType, "Multilabels", RETURN_TYPE_INSTANCE);
      break;
    case VW::prediction_type_t::prob:
      field = env->GetStaticFieldID(clVWReturnType, "Prob", RETURN_TYPE_INSTANCE);
      break;
    case VW::prediction_type_t::scalar:
      field = env->GetStaticFieldID(clVWReturnType, "Scalar", RETURN_TYPE_INSTANCE);
      break;
    case VW::prediction_type_t::scalars:
      field = env->GetStaticFieldID(clVWReturnType, "Scalars", RETURN_TYPE_INSTANCE);
      break;
    case VW::prediction_type_t::decision_probs:
      field = env->GetStaticFieldID(clVWReturnType, "DecisionProbs", RETURN_TYPE_INSTANCE);
      break;
    default:
      field = env->GetStaticFieldID(clVWReturnType, "Unknown", RETURN_TYPE_INSTANCE);
  }

  return env->GetStaticObjectField(clVWReturnType, field);
}
