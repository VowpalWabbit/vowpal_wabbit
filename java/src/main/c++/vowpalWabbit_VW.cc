#include "vowpalWabbit_VW.h"

#include "vw/core/vw.h"

JNIEXPORT jstring JNICALL Java_vowpalWabbit_VW_version(JNIEnv *env, jclass obj)
{
  return env->NewStringUTF(PACKAGE_VERSION);
}