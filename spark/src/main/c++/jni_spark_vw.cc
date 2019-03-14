#include "jni_spark_vw.h"
#include "vw_exception.h"
#include "best_constant.h"
#include "vector_io_buf.h"
#include "util.h"

StringGuard::StringGuard(JNIEnv *env, jstring source) : _env(env), _source(source), _cstr(nullptr)
{ _cstr = _env->GetStringUTFChars(source, 0);
}

StringGuard::~StringGuard()
{ if (_cstr) {
    _env->ReleaseStringUTFChars(_source, _cstr);
    _env->DeleteLocalRef(_source);
  }
}

const char* StringGuard::c_str()
{ return _cstr; }

// VW 
JNIEXPORT jlong JNICALL Java_vowpalwabbit_spark_VowpalWabbitNative_initialize
  (JNIEnv *env, jclass, jstring args)
{ StringGuard g_args(env, args);

  try
  { return (jlong)VW::initialize(g_args.c_str());
  }
  catch(...)
  { rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT jbyteArray JNICALL Java_vowpalwabbit_spark_VowpalWabbitNative_getModel
  (JNIEnv *env, jclass, jlong vwPtr)
{
  auto all = (vw*)vwPtr;
  try
  { // save in stl::vector
    vector_io_buf buffer;
    VW::save_predictor(*all, buffer);

    // copy to Java
    jbyteArray ret = env->NewByteArray(buffer._buffer.size());
    env->SetByteArrayRegion (ret, 0, buffer._buffer.size(), (const jbyte*)&buffer._buffer[0]);
    return ret;
  }
  catch(...)
  { rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT void JNICALL Java_vowpalwabbit_spark_VowpalWabbitNative_endPass
  (JNIEnv *env, jclass, jlong vwPtr)
{
  auto all = (vw*)vwPtr;

  try
  { all->l->end_pass();
    VW::sync_stats(*all);
  }
  catch(...)
  { rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT void JNICALL Java_vowpalwabbit_spark_VowpalWabbitNative_finish
  (JNIEnv *env, jclass, jlong vwPtr)
{
  auto all = (vw*)vwPtr;

  try
  {  VW::finish(*all);
  }
  catch(...)
  { rethrow_cpp_exception_as_java_exception(env);
  }
}

// VW Example
JNIEXPORT jlong JNICALL Java_vowpalwabbit_spark_VowpalWabbitExample_initialize
  (JNIEnv *env, jclass, jlong vwPtr, jboolean isEmpty)
{
  auto all = (vw*)vwPtr;

  try
  { example* ex = VW::alloc_examples(0, 1);

    if (isEmpty) {
      char empty = '\0';
      VW::read_line(*all, ex, &empty);
    }
    else 
        all->p->lp.default_label(&ex->l);

    // VW::setup_example(*all, ex); 

    return (jlong)new VowpalWabbitExampleWrapper(all, ex);
  }
  catch(...)
  { rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT jlong JNICALL Java_vowpalwabbit_spark_VowpalWabbitExample_finish
  (JNIEnv *env, jclass, jlong examplePtr)
{
  auto exWrapper = (VowpalWabbitExampleWrapper*)examplePtr;
  vw* all = exWrapper->_all;
  example* ex = exWrapper->_example;

  try
  { VW::dealloc_example(all->p->lp.delete_label, *ex);
    ::free_it(ex);
  }
  catch(...)
  { rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT void JNICALL Java_vowpalwabbit_spark_VowpalWabbitExample_clear
  (JNIEnv *env, jclass, jlong examplePtr)
{
  auto exWrapper = (VowpalWabbitExampleWrapper*)examplePtr;
  vw* all = exWrapper->_all;
  example* ex = exWrapper->_example;

  try
  { VW::empty_example(*all, *ex);
    all->p->lp.default_label(&ex->l);
    // VW::setup_example(*all, ex); 
  }
  catch(...)
  { rethrow_cpp_exception_as_java_exception(env);
  }
}

void addNamespaceIfNotExists(vw* all, example* ex, char ns)
{ for (unsigned char ns_existing : ex->indices)
    if (ns_existing == ns)
        return;

    ex->indices.push_back(ns);
}

JNIEXPORT void JNICALL Java_vowpalwabbit_spark_VowpalWabbitExample_addToNamespaceDense
  (JNIEnv *env, jclass, jlong examplePtr, jchar ns, jint weight_index_base, jdoubleArray values)
{
  auto exWrapper = (VowpalWabbitExampleWrapper*)examplePtr;
  vw* all = exWrapper->_all;
  example* ex = exWrapper->_example;


  try
  { addNamespaceIfNotExists(all, ex, ns);

    auto features = ex->feature_space + ns;

    // TODO: need to implement guard to get finally covered
    int size = env->GetArrayLength(values);
    double* values0 = (double*)env->GetPrimitiveArrayCritical(values, nullptr);

    // pre-allocate
    features->values.resize(features->values.end() - features->values.begin() + size);
    features->indicies.resize(features->indicies.end() - features->indicies.begin() + size);

    double* values_itr = values0;
    double* values_end = values0 + size;
    for (; values_itr != values_end; ++values_itr, ++weight_index_base)
    { float x = *values_itr;
      if (x != 0)
      { features->values.push_back_unchecked(x);
        features->indicies.push_back_unchecked(weight_index_base);
      }
    }

    env->ReleasePrimitiveArrayCritical(values, values0, JNI_ABORT);
  }
  catch(...)
  { rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT void JNICALL Java_vowpalwabbit_spark_VowpalWabbitExample_addToNamespaceSparse
  (JNIEnv *env, jclass, jlong examplePtr, jchar ns, jintArray indices, jdoubleArray values)
{
  auto exWrapper = (VowpalWabbitExampleWrapper*)examplePtr;
  vw* all = exWrapper->_all;
  example* ex = exWrapper->_example;

  try
  { addNamespaceIfNotExists(all, ex, ns);

    auto features = ex->feature_space + ns;

    // TODO: need to implement guard to get finally covered
    int size = env->GetArrayLength(indices);
    int* indices0 = (int*)env->GetPrimitiveArrayCritical(indices, nullptr);
    double* values0 = (double*)env->GetPrimitiveArrayCritical(values, nullptr);

    // pre-allocate
    features->values.resize(features->values.end() - features->values.begin() + size);
    features->indicies.resize(features->indicies.end() - features->indicies.begin() + size);

    int* indices_itr = indices0;
    int* indices_end = indices0 + size;
    double* values_itr = values0;
    for (; indices_itr != indices_end; ++indices_itr, ++values_itr)
    { float x = *values_itr;
      if (x != 0)
      { features->values.push_back_unchecked(x);
        features->indicies.push_back_unchecked(*indices_itr);
      }
    }

    env->ReleasePrimitiveArrayCritical(values, values0, JNI_ABORT);
    env->ReleasePrimitiveArrayCritical(indices, indices0, JNI_ABORT);
  }
  catch(...)
  { rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT void JNICALL Java_vowpalwabbit_spark_VowpalWabbitExample_setLabel
  (JNIEnv *env, jclass, jlong examplePtr, jfloat label)
{
  auto exWrapper = (VowpalWabbitExampleWrapper*)examplePtr;
  vw* all = exWrapper->_all;
  example* ex = exWrapper->_example;

  try
  {  label_data* ld = (label_data*)&ex->l;
     ld->label = label;

     count_label(all->sd, ld->label);
  }
  catch(...)
  { rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT void JNICALL Java_vowpalwabbit_spark_VowpalWabbitExample_setLabelWithWeight
  (JNIEnv *env, jclass, jlong examplePtr, jfloat weight, jfloat label)
{
  auto exWrapper = (VowpalWabbitExampleWrapper*)examplePtr;
  vw* all = exWrapper->_all;
  example* ex = exWrapper->_example;

  try
  {  label_data* ld = (label_data*)&ex->l;
     ld->label = label;
     ld->weight = weight;

     count_label(all->sd, ld->label);
  }
  catch(...)
  { rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT jobject JNICALL Java_vowpalwabbit_spark_VowpalWabbitExample_getPrediction
  (JNIEnv *env, jclass, jlong examplePtr)
{
  auto exWrapper = (VowpalWabbitExampleWrapper*)examplePtr;
  vw* all = exWrapper->_all;
  example* ex = exWrapper->_example;

 jclass predClass;
 jmethodID ctr;
 switch (all->l->pred_type)
  {
    case prediction_type::prediction_type_t::scalar:
        predClass = env->FindClass("vowpalwabbit/spark/prediction/ScalarPrediction");
        ctr = env->GetMethodID(predClass, "<init>", "(FF)V");

        return env->NewObject(predClass, ctr, 
            VW::get_prediction(ex),
            ex->confidence);
    
    case prediction_type::prediction_type_t::prob:
        predClass = env->FindClass("java/lang/Float");
        ctr = env->GetMethodID(predClass, "<init>", "(F)V");

        return env->NewObject(predClass, ctr, 
            ex->pred.prob);
         
 /*       
    case prediction_type::prediction_type_t::action_probs:
    case prediction_type::prediction_type_t::action_scores:
    case prediction_type::prediction_type_t::multiclass:
    case prediction_type::prediction_type_t::multilabels:
    case prediction_type::prediction_type_t::scalars:*/
    default:
      return nullptr;
  }   
}

JNIEXPORT void JNICALL Java_vowpalwabbit_spark_VowpalWabbitExample_learn
  (JNIEnv *env, jclass, jlong examplePtr)
{
  auto exWrapper = (VowpalWabbitExampleWrapper*)examplePtr;
  vw* all = exWrapper->_all;
  example* ex = exWrapper->_example;

  try
  { VW::setup_example(*all, ex); 
  
    all->learn(*ex);
    
    // as this is not a ring-based example it is not free'd
    LEARNER::as_singleline(all->l)->finish_example(*all, *ex);
  }
  catch(...)
  { rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT void JNICALL Java_vowpalwabbit_spark_VowpalWabbitExample_predict
  (JNIEnv *env, jclass, jlong examplePtr)
{
  auto exWrapper = (VowpalWabbitExampleWrapper*)examplePtr;
  vw* all = exWrapper->_all;
  example* ex = exWrapper->_example;

  try
  { VW::setup_example(*all, ex); 

    all->predict(*ex);
    
    // as this is not a ring-based example it is not free'd
    LEARNER::as_singleline(all->l)->finish_example(*all, *ex);
  }
  catch(...)
  { rethrow_cpp_exception_as_java_exception(env);
  }
}