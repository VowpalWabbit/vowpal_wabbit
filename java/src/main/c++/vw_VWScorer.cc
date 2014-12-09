#include "../../../../vowpalwabbit/parser.h"
#include "../../../../vowpalwabbit/vw.h"
#include "vw_VWScorer.h"

vw* vw;

JNIEXPORT void JNICALL Java_vw_VWScorer_initialize
  (JNIEnv *env, jobject obj, jstring command) {
    vw = VW::initialize(env->GetStringUTFChars(command, NULL));
}

JNIEXPORT jfloat JNICALL Java_vw_VWScorer_getPrediction
  (JNIEnv *env, jobject obj, jstring example_string) {
    example *vec2 = VW::read_example(*vw, env->GetStringUTFChars(example_string, NULL));
    vw->l->predict(*vec2);
    float prediction;
    if (vw->p->lp.parse_label == simple_label.parse_label)
        prediction = vec2->pred.scalar;
    else
        prediction = vec2->pred.multiclass;
     VW::finish_example(*vw, vec2);
    return prediction;
}

JNIEXPORT void JNICALL Java_vw_VWScorer_closeInstance
  (JNIEnv *env, jobject obj) {
    VW::finish(*vw);
}
