#include "../../../../vowpalwabbit/vw.h"
#include "vw_VowpalWabbit.h"

JNIEXPORT jstring JNICALL Java_vw_VowpalWabbit_version(JNIEnv *env, jclass obj)
{ return env->NewStringUTF(PACKAGE_VERSION);
}
