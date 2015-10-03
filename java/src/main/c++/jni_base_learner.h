#ifndef VW_BASE_LEARNER_H
#define VW_BASE_LEARNER_H

#include <jni.h>

void throw_java_exception(JNIEnv *env, const char* name, const char* msg);
void rethrow_cpp_exception_as_java_exception(JNIEnv *env);

template<class T>
T base_predict(
        JNIEnv *env,
        jobject obj,
        jstring example_string,
        jboolean learn,
        jlong vwPtr,
        const std::function<T(example*)> &predictor) {
    T result;
    try {
        vw* vwInstance = (vw*)vwPtr;
        const char *utf_string = env->GetStringUTFChars(example_string, NULL);
        example *vec = VW::read_example(*vwInstance, utf_string);

        if (learn)
            vwInstance->l->learn(*vec);
        else
            vwInstance->l->predict(*vec);

        result = predictor(vec);

        VW::finish_example(*vwInstance, vec);
        env->ReleaseStringUTFChars(example_string, utf_string);
        env->DeleteLocalRef(example_string);
        return result;
    } catch (...) {
        rethrow_cpp_exception_as_java_exception(env);
    }
    return result;
}

#endif // VW_BASE_LEARNER_H
