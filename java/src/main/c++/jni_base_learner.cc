#include "vw.h"
#include "vw_exception.h"

#include "jni_base_learner.h"

example* read_example(JNIEnv* env, jstring example_string, vw* vwInstance)
{
  const char* utf_string = env->GetStringUTFChars(example_string, NULL);
  example* ex = read_example(utf_string, vwInstance);

  env->ReleaseStringUTFChars(example_string, utf_string);
  env->DeleteLocalRef(example_string);

  return ex;
}

example* read_example(const char* example_string, vw* vwInstance)
{
  return VW::read_example(*vwInstance, example_string);
}
