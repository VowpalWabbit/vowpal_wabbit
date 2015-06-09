#include "../../../../vowpalwabbit/parser.h"
#include "../../../../vowpalwabbit/vw.h"
#include "vw_VWScorer.h"

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
        throw_java_exception(env, "java/lang/Exception", e.what());
    }
    catch (...) {
        throw_java_exception(env, "java/lang/Error", "Unidentified exception => "
                                   "rethrow_cpp_exception_as_java_exception "
                                   "may require some completion...");
    }
}

vw* getVW(jlong vwPtr) {
    return (vw*) vwPtr;
}

JNIEXPORT jlong JNICALL Java_vw_VWScorer_initialize(JNIEnv *env, jobject obj, jstring command) {
    jlong vwPtr;
    try {
        vwPtr = (jlong) VW::initialize(env->GetStringUTFChars(command, NULL));
    }
    catch(...) {
        rethrow_cpp_exception_as_java_exception(env);
    }
    return vwPtr;
}

JNIEXPORT jfloat JNICALL Java_vw_VWScorer_getPrediction(JNIEnv *env, jobject obj, jstring example_string, jboolean learn, jlong vwPtr) {
    float prediction;
    try {
        vw* vw = getVW(vwPtr);
        const char *utf_string = env->GetStringUTFChars(example_string, NULL);
        example *vec2 = VW::read_example(*vw, utf_string);
        if (learn)
            vw->l->learn(*vec2);
        else
            vw->l->predict(*vec2);
        if (vw->p->lp.parse_label == simple_label.parse_label)
            prediction = vec2->pred.scalar;
        else
            prediction = vec2->pred.multiclass;

        VW::finish_example(*vw, vec2);
        env->ReleaseStringUTFChars(example_string, utf_string);
        env->DeleteLocalRef(example_string);
    }
    catch (...) {
        rethrow_cpp_exception_as_java_exception(env);
    }
    return prediction;
}

JNIEXPORT jfloatArray JNICALL Java_vw_VWScorer_getPredictions(JNIEnv *env, jobject obj, jobjectArray examples, jboolean learn, jlong vwPtr) {
    size_t len = env->GetArrayLength(examples);
    float* predictions = new float[len];
    for (size_t i=0; i<len; ++i) {
        jstring example = (jstring)env->GetObjectArrayElement(examples, i);
        predictions[i] = Java_vw_VWScorer_getPrediction(env, obj, example, learn, vwPtr);
        env->DeleteLocalRef(example);
    }
    jfloatArray jPredictions = env->NewFloatArray(len);
    env->SetFloatArrayRegion(jPredictions, 0, len, predictions);

    free(predictions);
    env->DeleteLocalRef(examples);
    return jPredictions;
}

JNIEXPORT void JNICALL Java_vw_VWScorer_closeInstance(JNIEnv *env, jobject obj, jlong vwPtr) {
    try {
        VW::finish(*getVW(vwPtr));
    }
    catch(...) {
        rethrow_cpp_exception_as_java_exception(env);
    }
}
