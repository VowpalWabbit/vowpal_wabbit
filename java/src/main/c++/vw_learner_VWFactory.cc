#include "../../../../vowpalwabbit/vw.h"
#include "jni_base_learner.h"
#include "vw_learner_VWFactory.h"

#define returnType "vw/learner/VWFactory$VWReturnType"
#define returnTypeInstance "Lvw/learner/VWFactory$VWReturnType;"

JNIEXPORT jlong JNICALL Java_vw_learner_VWFactory_initialize(JNIEnv *env, jobject obj, jstring command)
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

JNIEXPORT void JNICALL Java_vw_learner_VWFactory_closeInstance(JNIEnv *env, jobject obj, jlong vwPtr)
{ try
  { VW::finish(*((vw*)vwPtr));
  }
  catch(...)
  { rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT jobject JNICALL Java_vw_learner_VWFactory_getReturnType(JNIEnv *env, jobject obj, jlong vwPtr)
{
    jclass clVWReturnType = env->FindClass(returnType);
    jfieldID field;
    vw* vwInstance = (vw*)vwPtr;
    if (vwInstance->p->lp.parse_label == simple_label.parse_label) {
        if (vwInstance->lda > 0)
            field = env->GetStaticFieldID(clVWReturnType , "VWFloatArrayType", returnTypeInstance);
        else
            field = env->GetStaticFieldID(clVWReturnType , "VWFloatType", returnTypeInstance);
    }
    else if (vwInstance->p->lp.parse_label == MULTILABEL::multilabel.parse_label)
        field = env->GetStaticFieldID(clVWReturnType , "VWIntArrayType", returnTypeInstance);
    // This is really bad and we need to figure out why this is not working
    /*else if (vwInstance->p->lp.parse_label == MULTICLASS::mc_label.parse_label)
        field = env->GetStaticFieldID(clVWReturnType , "VWIntType", returnTypeInstance);
    else
        field = env->GetStaticFieldID(clVWReturnType , "Unknown", returnTypeInstance);
        */
    else
        field = env->GetStaticFieldID(clVWReturnType , "VWIntType", returnTypeInstance);
    return env->GetStaticObjectField(clVWReturnType, field);
}

