#include "../../../../vowpalwabbit/parser.h"
#include "../../../../vowpalwabbit/vw.h"
#include "vw_errors.h"
#include "vw_VW.h"

JNIEXPORT jstring JNICALL Java_vw_VW_version(JNIEnv *env, jclass obj) {
    return env->NewStringUTF(PACKAGE_VERSION);
}

JNIEXPORT jfloat JNICALL Java_vw_VW_predictFloat(JNIEnv *env, jobject obj, jstring example_string, jboolean learn, jlong vwPtr) {
    float prediction = 0.0f;
    try {
        vw* vwInstance = (vw*)vwPtr;
        const char *utf_string = env->GetStringUTFChars(example_string, NULL);
        example *vec2 = VW::read_example(*vwInstance, utf_string);

        if (learn)
            vwInstance->l->learn(*vec2);
        else
            vwInstance->l->predict(*vec2);

        if (vwInstance->p->lp.parse_label == simple_label.parse_label)
            prediction = vec2->pred.scalar;
        else
            prediction = vec2->pred.multiclass;

        VW::finish_example(*vwInstance, vec2);
        env->ReleaseStringUTFChars(example_string, utf_string);
        env->DeleteLocalRef(example_string);
    }
    catch (...) {
        rethrow_cpp_exception_as_java_exception(env);
    }
    return prediction;
}
