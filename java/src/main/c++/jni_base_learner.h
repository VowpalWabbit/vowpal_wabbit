#ifndef VW_BASE_LEARNER_H
#define VW_BASE_LEARNER_H

#include <jni.h>

void throw_java_exception(JNIEnv *env, const char* name, const char* msg);
void rethrow_cpp_exception_as_java_exception(JNIEnv *env);

// It would appear that after reading posts like
// http://stackoverflow.com/questions/6458612/c0x-proper-way-to-receive-a-lambda-as-parameter-by-reference
// and
// http://stackoverflow.com/questions/3203305/write-a-function-that-accepts-a-lambda-expression-as-argument
// it is more efficient to use another type parameter instead of std::function<T(example*)>
// but more difficult to read.
template<typename T, typename F>
T base_predict(
        JNIEnv *env,
        jobject obj,
        jstring example_string,
        jboolean learn,
        jlong vwPtr,
        const F &predictor) {
    T result = 0;
    try {
        vw* vwInstance = (vw*)vwPtr;
        const char *utf_string = env->GetStringUTFChars(example_string, NULL);
        example *vec = VW::read_example(*vwInstance, utf_string);

        if (learn)
            vwInstance->l->learn(*vec);
        else
            vwInstance->l->predict(*vec);

        result = predictor(vec, env);

        VW::finish_example(*vwInstance, vec);
        env->ReleaseStringUTFChars(example_string, utf_string);
        env->DeleteLocalRef(example_string);
    } catch (...) {
        rethrow_cpp_exception_as_java_exception(env);
    }
    return result;
}

#endif // VW_BASE_LEARNER_H
