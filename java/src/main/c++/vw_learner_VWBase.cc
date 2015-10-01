#include "../../../../vowpalwabbit/parser.h"
#include "../../../../vowpalwabbit/vw.h"
#include "vw_errors.h"
#include "vw_learner_VWBase.h"


JNIEXPORT jlong JNICALL Java_vw_learner_VWBase_initialize(JNIEnv *env, jobject obj, jstring command) {
    jlong vwPtr = 0;
    try {
        vw* vwInstance = VW::initialize(env->GetStringUTFChars(command, NULL));
        vwPtr = (jlong)vwInstance;
        if (vwInstance->multilabel_prediction) {
            Java_vw_learner_VWBase_closeInstance(env, obj, vwPtr);
            vwPtr = 0;
            throw_java_exception(env, "vw/exception/IllegalVWInput", "VW JNI layer only supports simple and multiclass predictions");
        }
    }
    catch(...) {
        rethrow_cpp_exception_as_java_exception(env);
    }
    return vwPtr;
}

JNIEXPORT void JNICALL Java_vw_learner_VWBase_closeInstance(JNIEnv *env, jobject obj, jlong vwPtr) {
    try {
        VW::finish(*((vw*)vwPtr));
    }
    catch(...) {
        rethrow_cpp_exception_as_java_exception(env);
    }
}
