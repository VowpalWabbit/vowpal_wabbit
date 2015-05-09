#include "../../../../vowpalwabbit/parser.h"
#include "../../../../vowpalwabbit/vw.h"
#include "vw_VWScorer.h"

vw* vw;

void throw_java_exception(JNIEnv *env, const char* name, const char* msg){
     jclass jc = env->FindClass(name);
     if(jc) env->ThrowNew (jc, msg);
}

void rethrow_cpp_exception_as_java_exception(JNIEnv *env){
    try{
        throw;
    }
    catch(const std::bad_alloc& e) {
        throw_java_exception(env, "java/lang/OutOfMemoryError", e.what()); 
    }
    catch(const std::ios_base::failure& e){
        throw_java_exception(env, "java/io/IOException", e.what());  
    }
    catch(const std::exception& e){
        throw_java_exception(env, "java/lang/Error", e.what()); 
    }
    catch (...) {
        throw_java_exception(env, "java/lang/Error", "Unidentified exception => "
                                   "rethrow_cpp_exception_as_java_exception "
                                   "may require some completion...");
    }
}

float getPrediction(JNIEnv *env, example *vec2){
    float prediction;
    try{
        if (vw->p->lp.parse_label == simple_label.parse_label)
            prediction = vec2->pred.scalar;
        else
            prediction = vec2->pred.multiclass;

        VW::finish_example(*vw, vec2);
    }
    catch (...){
        rethrow_cpp_exception_as_java_exception(env);
    }
    return prediction;
}

JNIEXPORT void JNICALL Java_vw_VWScorer_initialize (JNIEnv *env, jobject obj, jstring command) {
    try{
        vw = VW::initialize(env->GetStringUTFChars(command, NULL));
    }
    catch(...){
        rethrow_cpp_exception_as_java_exception(env);
    }
}

JNIEXPORT jfloat JNICALL Java_vw_VWScorer_doLearnAndGetPrediction (JNIEnv *env, jobject obj, jstring example_string) {
    const char *utf_string = env->GetStringUTFChars(example_string, NULL);
    example *vec2 = VW::read_example(*vw, utf_string);
    vw->l->learn(*vec2);
    float prediction = getPrediction(env, vec2);
    env->ReleaseStringUTFChars(example_string, utf_string);
    env->DeleteLocalRef(example_string);
    return prediction;
}

JNIEXPORT jfloat JNICALL Java_vw_VWScorer_getPrediction (JNIEnv *env, jobject obj, jstring example_string) {
    const char *utf_string = env->GetStringUTFChars(example_string, NULL);
    example *vec2 = VW::read_example(*vw, utf_string);
    vw->l->predict(*vec2);
    float prediction = getPrediction(env, vec2);
    env->ReleaseStringUTFChars(example_string, utf_string);
    env->DeleteLocalRef(example_string);
    return prediction; 
}

JNIEXPORT void JNICALL Java_vw_VWScorer_closeInstance (JNIEnv *env, jobject obj) {
    try{
    VW::finish(*vw);
    }
    catch(...){
        rethrow_cpp_exception_as_java_exception(env);
    }
}
