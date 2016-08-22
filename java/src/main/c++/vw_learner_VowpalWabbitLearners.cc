#include "../../../../vowpalwabbit/vw.h"
#include "jni_base_learner.h"
#include "vw_learner_VowpalWabbitLearners.h"

#define RETURN_TYPE "vw/learner/VowpalWabbitLearners$VowpalWabbitReturnType"
#define RETURN_TYPE_INSTANCE "L" RETURN_TYPE ";"

JNIEXPORT jlong JNICALL Java_vw_learner_VowpalWabbitLearners_initialize(JNIEnv *env, jobject obj, jstring command)
{ jlong vwPtr = 0;
  try
  { vw* vwInstance = VW::initialize(env->GetStringUTFChars(command, NULL));
    vwPtr = (jlong)vwInstance;
  }
  catch(...)
  { rethrow_cpp_exception_as_java_exception(env);
  }
  return vwPtr;
}

JNIEXPORT void JNICALL Java_vw_learner_VowpalWabbitLearners_closeInstance(JNIEnv *env, jobject obj, jlong vwPtr)
{ try
  { VW::finish(*((vw*)vwPtr));
  }
  catch(...)
  { rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT jobject JNICALL Java_vw_learner_VowpalWabbitLearners_getReturnType(JNIEnv *env, jobject obj, jlong vwPtr)
{ jclass clVowpalWabbitReturnType = env->FindClass(RETURN_TYPE);
  jfieldID field;
  vw* vwInstance = (vw*)vwPtr;
  switch (vwInstance->l->pred_type)
  { case prediction_type::prediction_type_t::action_scores:
      field = env->GetStaticFieldID(clVowpalWabbitReturnType , "ActionScores", RETURN_TYPE_INSTANCE);
      break;
    case prediction_type::prediction_type_t::multiclass:
      field = env->GetStaticFieldID(clVowpalWabbitReturnType , "Multiclass", RETURN_TYPE_INSTANCE);
      break;
    case prediction_type::prediction_type_t::multilabels:
      field = env->GetStaticFieldID(clVowpalWabbitReturnType , "Multilabels", RETURN_TYPE_INSTANCE);
      break;
    case prediction_type::prediction_type_t::prob:
      field = env->GetStaticFieldID(clVowpalWabbitReturnType , "Prob", RETURN_TYPE_INSTANCE);
      break;
    case prediction_type::prediction_type_t::scalar:
      field = env->GetStaticFieldID(clVowpalWabbitReturnType , "Scalar", RETURN_TYPE_INSTANCE);
      break;
    case prediction_type::prediction_type_t::scalars:
      field = env->GetStaticFieldID(clVowpalWabbitReturnType , "Scalars", RETURN_TYPE_INSTANCE);
      break;
  }

  return env->GetStaticObjectField(clVowpalWabbitReturnType, field);
}
