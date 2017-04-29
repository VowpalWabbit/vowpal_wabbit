#include <string>
#include "vowpalWabbit_learner_VWMulticlassLearner.h"
#include "../../../../vowpalwabbit/vw.h"
#include "jni_base_learner.h"

jint multiclass_predictor(example* vec, JNIEnv *env){ return vec->pred.multiclass; }

JNIEXPORT jint JNICALL Java_vowpalWabbit_learner_VWMulticlassLearner_predict(JNIEnv *env, jobject obj, jstring example_string, jboolean learn, jlong vwPtr)
{ return base_predict<jint>(env, example_string, learn, vwPtr, multiclass_predictor);
}

JNIEXPORT jint JNICALL Java_vowpalWabbit_learner_VWMulticlassLearner_predictMultiline(JNIEnv *env, jobject obj, jobjectArray example_strings, jboolean learn, jlong vwPtr)
{ return base_predict<jint>(env, example_strings, learn, vwPtr, multiclass_predictor);
}

/*
 * private multiline prediction utility
 * predict and annotates for multiline example string arrays
 * results will be stored as example ptrs in the example array passed in
 */
void _predict_for_multilines(JNIEnv *env, jobject obj, jobjectArray example_strings, jboolean learn, jlong vwPtr, example** ex_array)
{ vw* vwInstance = (vw*)vwPtr;
  int example_count = env->GetArrayLength(example_strings);
  
  // first pass to process all examples without giving final predictions
  for (int i=0; i<example_count; i++) {
    jstring example_string = (jstring) (env->GetObjectArrayElement(example_strings, i));
    example* ex = read_example(env, example_string, vwInstance);
    base_predict<jint>(env, ex, learn, vwInstance, multiclass_predictor, false);
    ex_array[i] = ex;
  }
  
  // release JVM references to examples
  env->DeleteLocalRef(example_strings);

  // close out examples
  example* ex = read_example("\0", vwInstance);
  base_predict<jint>(env, ex, learn, vwInstance, multiclass_predictor, false);
  
  return; 
}

JNIEXPORT jintArray JNICALL Java_vowpalWabbit_learner_VWMulticlassLearner_predictForAllLines(JNIEnv *env, jobject obj, jobjectArray example_strings, jboolean learn, jlong vwPtr)
{ int example_count = env->GetArrayLength(example_strings);
  example** ex_array = new example*[example_count]; 

  // annotate examples for inputs
  _predict_for_multilines(env, obj, example_strings, learn, vwPtr, ex_array);
  
  // second pass to collect all predictions in int 
  jint* pred_c_array = new jint[example_count];
  for (int i=0; i<example_count; i++) {
    pred_c_array[i] = ex_array[i]->pred.multiclass;
  }

  // alloc pred_j_array
  jintArray pred_j_array = env->NewIntArray(example_count);
  env->SetIntArrayRegion(pred_j_array, 0, example_count, pred_c_array);

  // release allocated resources
  delete[] pred_c_array;
  delete[] ex_array;
  return pred_j_array;
}

JNIEXPORT jobjectArray JNICALL Java_vowpalWabbit_learner_VWMulticlassLearner_predictNamedLabelsForAllLines(JNIEnv *env, jobject obj, jobjectArray example_strings, jboolean learn, jlong vwPtr)
{ vw* vwInstance = (vw*)vwPtr;
  int example_count = env->GetArrayLength(example_strings);
  example** ex_array = new example*[example_count];
  
  // annotate examples for inputs
  _predict_for_multilines(env, obj, example_strings, learn, vwPtr, ex_array);
  
  // second pass to collect all pretty string predictions from annotated examples 
  jobjectArray pred_j_str_array = env->NewObjectArray(example_count, env->FindClass("java/lang/String"), NULL);
  for (int i=0; i<example_count; i++) {
    jstring pretty_pred_str = NULL;
    if (vwInstance->sd->ldict) {
      // if name labels were provided, use the named labels from model
      substring ss = vwInstance->sd->ldict->get(ex_array[i]->pred.multiclass); 
      pretty_pred_str = env->NewStringUTF(std::string(ss.begin, ss.end-ss.begin).c_str()); 
    } else {
      // else use the string value of the multiclass prediction index as an output
      pretty_pred_str = env->NewStringUTF(std::to_string(ex_array[i]->pred.multiclass).c_str());
    }
    env->SetObjectArrayElement(pred_j_str_array, i, pretty_pred_str);
  }
  
  // release allocated resources
  delete[] ex_array;
  return pred_j_str_array;
}
