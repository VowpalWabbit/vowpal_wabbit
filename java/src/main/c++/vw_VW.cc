#include "../../../../vowpalwabbit/parser.h"
#include "../../../../vowpalwabbit/vw.h"
#include "vw_VW.h"

void throw_java_exception(JNIEnv *env, const char* name, const char* msg) {
     jclass jc = env->FindClass(name);
     if (jc)
        env->ThrowNew (jc, msg);
}

void rethrow_cpp_exception_as_java_exception(JNIEnv *env) {
    try {
        throw;
    }
    catch(const std::bad_alloc& e) {
        throw_java_exception(env, "java/lang/OutOfMemoryError", e.what());
    }
    catch(const std::ios_base::failure& e) {
        throw_java_exception(env, "java/io/IOException", e.what());
    }
    catch(const std::exception& e) {
        const char* what = e.what();
        std::string what_str = std::string(what);
        // It would appear that this text has changed between different boost variants
        std::string prefix1("unrecognised option");
        std::string prefix2("unknown option");

        if (what_str.substr(0, prefix1.size()) == prefix1 ||
            what_str.substr(0, prefix2.size()) == prefix2)
            throw_java_exception(env, "java/lang/IllegalArgumentException", what);
        else
            throw_java_exception(env, "java/lang/Exception", what);
    }
    catch (...) {
        throw_java_exception(env, "java/lang/Error", "Unidentified exception => "
                                   "rethrow_cpp_exception_as_java_exception "
                                   "may require some completion...");
    }
}

JNIEXPORT jlong JNICALL Java_vw_VW_initialize(JNIEnv *env, jobject obj, jstring command) {
    jlong vwPtr = 0;
    try {
        vwPtr = (jlong) VW::initialize(env->GetStringUTFChars(command, NULL));
    }
    catch(...) {
        rethrow_cpp_exception_as_java_exception(env);
    }
    return vwPtr;
}

JNIEXPORT jfloat JNICALL Java_vw_VW_predict(JNIEnv *env, jobject obj, jstring example_string, jboolean learn, jlong vwPtr) {
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

JNIEXPORT void JNICALL Java_vw_VW_closeInstance(JNIEnv *env, jobject obj, jlong vwPtr) {
    try {
        VW::finish(*((vw*)vwPtr));
    }
    catch(...) {
        rethrow_cpp_exception_as_java_exception(env);
    }
}
