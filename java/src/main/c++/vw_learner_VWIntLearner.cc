#include "../../../../vowpalwabbit/parser.h"
#include "../../../../vowpalwabbit/vw.h"
#include "jni_base_learner.h"
#include "vw_learner_VWIntLearner.h"

jint intPredictor(vw* vwInstance, example* vec, JNIEnv *env)
{ jint result = vec->pred.multiclass;
    VW::finish_example(*vwInstance, vec);
    return result;
}

JNIEXPORT jint JNICALL Java_vw_learner_VWIntLearner_predict(JNIEnv *env, jobject obj, jstring example_string, jboolean learn, jlong vwPtr)
{ return base_predict<jint>(env, obj, example_string, learn, vwPtr, intPredictor);
}
