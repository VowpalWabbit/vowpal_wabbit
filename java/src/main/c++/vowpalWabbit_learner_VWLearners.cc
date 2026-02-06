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
    const char* utf_string = env->GetStringUTFChars(command, NULL);
    VW::workspace* vwInstance = VW::initialize(utf_string);
    env->ReleaseStringUTFChars(command, utf_string);
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
    if (vwInstance->runtime_config.numpasses > 1)
    {
      vwInstance->runtime_state.do_reset_source = true;
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
    vwInstance->finish();
    delete vwInstance;
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

JNIEXPORT jboolean JNICALL Java_vowpalWabbit_learner_VWLearners_isMultiline(JNIEnv* env, jclass obj, jlong vwPtr)
{
  try
  {
    VW::workspace* vwInstance = (VW::workspace*)vwPtr;
    return vwInstance->l->is_multiline() ? JNI_TRUE : JNI_FALSE;
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
  }
  return JNI_FALSE;
}

JNIEXPORT jobject JNICALL Java_vowpalWabbit_learner_VWLearners_getReturnType(JNIEnv* env, jclass obj, jlong vwPtr)
{
  jclass clVWReturnType = env->FindClass(RETURN_TYPE);
  jfieldID field;
  VW::workspace* vwInstance = (VW::workspace*)vwPtr;
  switch (vwInstance->l->get_output_prediction_type())
  {
    case VW::prediction_type_t::ACTION_PROBS:
      field = env->GetStaticFieldID(clVWReturnType, "ActionProbs", RETURN_TYPE_INSTANCE);
      break;
    case VW::prediction_type_t::ACTION_SCORES:
      field = env->GetStaticFieldID(clVWReturnType, "ActionScores", RETURN_TYPE_INSTANCE);
      break;
    case VW::prediction_type_t::MULTICLASS:
      field = env->GetStaticFieldID(clVWReturnType, "Multiclass", RETURN_TYPE_INSTANCE);
      break;
    case VW::prediction_type_t::MULTILABELS:
      field = env->GetStaticFieldID(clVWReturnType, "Multilabels", RETURN_TYPE_INSTANCE);
      break;
    case VW::prediction_type_t::PROB:
      field = env->GetStaticFieldID(clVWReturnType, "Prob", RETURN_TYPE_INSTANCE);
      break;
    case VW::prediction_type_t::SCALAR:
      field = env->GetStaticFieldID(clVWReturnType, "Scalar", RETURN_TYPE_INSTANCE);
      break;
    case VW::prediction_type_t::SCALARS:
      field = env->GetStaticFieldID(clVWReturnType, "Scalars", RETURN_TYPE_INSTANCE);
      break;
    case VW::prediction_type_t::DECISION_PROBS:
      field = env->GetStaticFieldID(clVWReturnType, "DecisionProbs", RETURN_TYPE_INSTANCE);
      break;
    case VW::prediction_type_t::ACTION_PDF_VALUE:
      field = env->GetStaticFieldID(clVWReturnType, "ActionPDFValue", RETURN_TYPE_INSTANCE);
      break;
    case VW::prediction_type_t::PDF:
      field = env->GetStaticFieldID(clVWReturnType, "PDF", RETURN_TYPE_INSTANCE);
      break;
    case VW::prediction_type_t::ACTIVE_MULTICLASS:
      field = env->GetStaticFieldID(clVWReturnType, "ActiveMulticlass", RETURN_TYPE_INSTANCE);
      break;
    case VW::prediction_type_t::NOPRED:
      field = env->GetStaticFieldID(clVWReturnType, "NoPred", RETURN_TYPE_INSTANCE);
      break;
    default:
      field = env->GetStaticFieldID(clVWReturnType, "Unknown", RETURN_TYPE_INSTANCE);
  }

  return env->GetStaticObjectField(clVWReturnType, field);
}
