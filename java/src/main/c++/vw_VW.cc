#include "../../../../vowpalwabbit/vw.h"
#include "vw_VW.h"

JNIEXPORT jstring JNICALL Java_vw_VW_version(JNIEnv *env, jclass obj)
{ return env->NewStringUTF(PACKAGE_VERSION);
}
