#include "../../../../vowpalwabbit/parser.h"
#include "../../../../vowpalwabbit/vw.h"
#include "vw_VWScorer.h"

vw* vw;

float getPrediction(JNIEnv *env, example *vec2){
    float prediction;
    if (vw->p->lp.parse_label == simple_label.parse_label)
        prediction = vec2->pred.scalar;
    else
        prediction = vec2->pred.multiclass;

    VW::finish_example(*vw, vec2);
    return prediction;
}

JNIEXPORT void JNICALL Java_vw_VWScorer_initialize (JNIEnv *env, jobject obj, jstring command) {
  
    vw = VW::initialize(env->GetStringUTFChars(command, NULL));
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

JNIEXPORT void JNICALL Java_vw_VWScorer_closeInstance
  (JNIEnv *env, jobject obj) {
    VW::finish(*vw);
}
