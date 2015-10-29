#include "../../../../vowpalwabbit/vw.h"
#include "jni_base_learner.h"
#include "vw_learner_VWFactory.h"

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

JNIEXPORT jobject JNICALL Java_vw_learner_VWFactory_getReturnType(JNIEnv *env, jobject obj, jlong vwPtr)
{ jclass clVWReturnType = env->FindClass("vw/learner/VWReturnType");
    jfieldID field = env->GetStaticFieldID(clVWReturnType , "Unknown", "Lvw/learner/VWReturnType;");
    try
    { vw* vwInstance = (vw*)vwPtr;
        if (vwInstance->p->lp.parse_label == simple_label.parse_label) {
            if (vwInstance->lda > 0)
                field = env->GetStaticFieldID(clVWReturnType , "VWFloatArrayType", "Lvw/learner/VWReturnType;");
            else
                field = env->GetStaticFieldID(clVWReturnType , "VWFloatType", "Lvw/learner/VWReturnType;");
        }
        else if (vwInstance->p->lp.parse_label == MULTILABEL::multilabel.parse_label)
            field = env->GetStaticFieldID(clVWReturnType , "VWIntArrayType", "Lvw/learner/VWReturnType;");
        //else if (vwInstance->p->lp.parse_label == MULTICLASS::mc_label.parse_label) // This is not good as we should find a better way of
        else
            field = env->GetStaticFieldID(clVWReturnType , "VWIntType", "Lvw/learner/VWReturnType;");
    }
    catch(...)
    { rethrow_cpp_exception_as_java_exception(env);
    }
    return env->GetStaticObjectField(clVWReturnType, field);
}

