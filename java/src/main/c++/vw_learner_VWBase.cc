#include "../../../../vowpalwabbit/vw.h"
#include "jni_base_learner.h"
#include "vw_learner_VWBase.h"

JNIEXPORT void JNICALL Java_vw_learner_VWBase_closeInstance(JNIEnv *env, jobject obj, jlong vwPtr)
{ try
  { VW::finish(*((vw*)vwPtr));
  }
  catch(...)
  { rethrow_cpp_exception_as_java_exception(env);
  }
}
