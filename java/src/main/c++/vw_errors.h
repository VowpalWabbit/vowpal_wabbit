#ifndef _Included_vw_errors
#define _Included_vw_errors

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

void throw_java_exception(JNIEnv *env, const char* name, const char* msg);
void rethrow_cpp_exception_as_java_exception(JNIEnv *env);

#ifdef __cplusplus
}
#endif
#endif
