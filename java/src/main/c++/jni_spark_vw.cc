#include "jni_spark_vw.h"

#include "util.h"
#include "vw/common/vw_exception.h"
#include "vw/config/cli_options_serializer.h"
#include "vw/config/options.h"
#include "vw/core/best_constant.h"
#include "vw/core/learner.h"
#include "vw/core/parse_example.h"
#include "vw/core/shared_data.h"
#include "vw/core/simple_label_parser.h"

#include <algorithm>
#include <exception>

jobject getJavaPrediction(JNIEnv* env, VW::workspace* all, example* ex);

// Guards
StringGuard::StringGuard(JNIEnv* env, jstring source) : _env(env), _source(source), _cstr(nullptr)
{
  _cstr = _env->GetStringUTFChars(source, 0);
}

StringGuard::~StringGuard()
{
  if (_cstr)
  {
    _env->ReleaseStringUTFChars(_source, _cstr);
    _env->DeleteLocalRef(_source);
  }
}

const char* StringGuard::c_str() { return _cstr; }

CriticalArrayGuard::CriticalArrayGuard(JNIEnv* env, jarray arr) : _env(env), _arr(arr), _arr0(nullptr)
{
  _arr0 = env->GetPrimitiveArrayCritical(arr, nullptr);
}

CriticalArrayGuard::~CriticalArrayGuard()
{
  if (_arr0) { _env->ReleasePrimitiveArrayCritical(_arr, _arr0, JNI_ABORT); }
}

void* CriticalArrayGuard::data() { return _arr0; }

// VW
JNIEXPORT jlong JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitNative_initialize(JNIEnv* env, jclass, jstring args)
{
  StringGuard g_args(env, args);

  try
  {
    return reinterpret_cast<jlong>(VW::initialize(g_args.c_str()));
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
    return 0;
  }
}

JNIEXPORT jlong JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitNative_initializeFromModel(
    JNIEnv* env, jclass, jstring args, jbyteArray model)
{
  StringGuard g_args(env, args);
  CriticalArrayGuard modelGuard(env, model);

  try
  {
    int size = env->GetArrayLength(model);
    auto* model0 = reinterpret_cast<const char*>(modelGuard.data());

    io_buf buffer;
    buffer.add_file(VW::io::create_buffer_view(model0, size));

    return reinterpret_cast<jlong>(VW::initialize(g_args.c_str(), &buffer));
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
    return 0;
  }
}

void populateMultiEx(JNIEnv* env, jobjectArray examples, VW::workspace& all, multi_ex& ex_coll)
{
  bool fieldIdInitialized = false;

  int length = env->GetArrayLength(examples);
  if (length > 0)
  {
    jobject jex = env->GetObjectArrayElement(examples, 0);

    jclass cls = env->GetObjectClass(jex);
    jfieldID fieldId = env->GetFieldID(cls, "nativePointer", "J");

    for (int i = 0; i < length; i++)
    {
      jex = env->GetObjectArrayElement(examples, i);

      // JavaObject VowpalWabbitExampleWrapper -> example*
      auto exWrapper = (VowpalWabbitExampleWrapper*)get_native_pointer(env, jex);
      VW::setup_example(all, exWrapper->_example);

      ex_coll.push_back(exWrapper->_example);
    }
  }
}

JNIEXPORT jobject JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitNative_learn(
    JNIEnv* env, jobject vwObj, jobjectArray examples)
{
  auto* all = reinterpret_cast<VW::workspace*>(get_native_pointer(env, vwObj));

  multi_ex ex_coll;
  try
  {
    populateMultiEx(env, examples, *all, ex_coll);

    all->learn(ex_coll);

    // as this is not a ring-based example it is not freed
    as_multiline(all->l)->finish_example(*all, ex_coll);

    // prediction is in the first example
    return getJavaPrediction(env, all, ex_coll[0]);
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
    return nullptr;
  }
}

JNIEXPORT jobject JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitNative_predict(
    JNIEnv* env, jobject vwObj, jobjectArray examples)
{
  auto* all = reinterpret_cast<VW::workspace*>(get_native_pointer(env, vwObj));

  multi_ex ex_coll;
  try
  {
    populateMultiEx(env, examples, *all, ex_coll);

    all->predict(ex_coll);

    // as this is not a ring-based example it is not freed
    as_multiline(all->l)->finish_example(*all, ex_coll);

    // prediction is in the first example
    return getJavaPrediction(env, all, ex_coll[0]);
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
    return nullptr;
  }
}

JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitNative_performRemainingPasses(JNIEnv* env, jobject vwObj)
{
  auto* all = reinterpret_cast<VW::workspace*>(get_native_pointer(env, vwObj));

  try
  {
    if (all->numpasses > 1)
    {
      all->do_reset_source = true;
      VW::start_parser(*all);
      VW::LEARNER::generic_driver(*all);
      VW::end_parser(*all);
    }
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT jbyteArray JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitNative_getModel(JNIEnv* env, jobject vwObj)
{
  auto* all = reinterpret_cast<VW::workspace*>(get_native_pointer(env, vwObj));

  try
  {  // save in stl::vector
    auto model_buffer = std::make_shared<std::vector<char>>();
    io_buf buffer;
    buffer.add_file(VW::io::create_vector_writer(model_buffer));
    VW::save_predictor(*all, buffer);

    // copy to Java
    jbyteArray ret = env->NewByteArray(model_buffer->size());
    CHECK_JNI_EXCEPTION(nullptr);

    env->SetByteArrayRegion(ret, 0, model_buffer->size(), (const jbyte*)&model_buffer->data()[0]);
    CHECK_JNI_EXCEPTION(nullptr);

    return ret;
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
    return nullptr;
  }
}

JNIEXPORT jobject JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitNative_getArguments(JNIEnv* env, jobject vwObj)
{
  auto* all = reinterpret_cast<VW::workspace*>(get_native_pointer(env, vwObj));

  // serialize the command line
  VW::config::cli_options_serializer serializer;
  for (auto const& option : all->options->get_all_options())
  {
    if (all->options->was_supplied(option->m_name)) { serializer.add(*option); }
  }

  // move it to Java
  // Note: don't keep serializer.str().c_str() around in some variable. it get's deleted after str() is de-allocated
  jstring args = env->NewStringUTF(serializer.str().c_str());
  CHECK_JNI_EXCEPTION(nullptr);

  jclass clazz = env->FindClass("org/vowpalwabbit/spark/VowpalWabbitArguments");
  CHECK_JNI_EXCEPTION(nullptr);

  jmethodID ctor = env->GetMethodID(clazz, "<init>", "(IILjava/lang/String;DD)V");
  CHECK_JNI_EXCEPTION(nullptr);

  return env->NewObject(clazz, ctor, all->num_bits, all->hash_seed, args, all->eta, all->power_t);
}

JNIEXPORT jstring JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitNative_getOutputPredictionType(
    JNIEnv* env, jobject vwObj)
{
  auto* all = reinterpret_cast<VW::workspace*>(get_native_pointer(env, vwObj));

  // produce string to avoid replication of enum types
  return env->NewStringUTF(std::string(VW::to_string(all->l->get_output_prediction_type())).c_str());
}

JNIEXPORT jobject JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitNative_getPerformanceStatistics(
    JNIEnv* env, jobject vwObj)
{
  auto* all = reinterpret_cast<VW::workspace*>(get_native_pointer(env, vwObj));

  long numberOfExamplesPerPass;
  double weightedExampleSum;
  double weightedLabelSum;
  double averageLoss;
  float bestConstant;
  float bestConstantLoss;
  long totalNumberOfFeatures;

  if (all->current_pass == 0)
    numberOfExamplesPerPass = all->sd->example_number;
  else
    numberOfExamplesPerPass = all->sd->example_number / all->current_pass;

  weightedExampleSum = all->sd->weighted_examples();
  weightedLabelSum = all->sd->weighted_labels;

  if (all->holdout_set_off)
    if (all->sd->weighted_labeled_examples > 0)
      averageLoss = all->sd->sum_loss / all->sd->weighted_labeled_examples;
    else
      averageLoss = 0;  // TODO should report NaN, but not clear how to do in platform independent manner
  else if ((all->sd->holdout_best_loss == FLT_MAX) || (all->sd->holdout_best_loss == FLT_MAX * 0.5))
    averageLoss = 0;  // TODO should report NaN, but not clear how to do in platform independent manner
  else
    averageLoss = all->sd->holdout_best_loss;

  VW::get_best_constant(*all->loss, *all->sd, bestConstant, bestConstantLoss);
  totalNumberOfFeatures = all->sd->total_features;

  jclass clazz = env->FindClass("org/vowpalwabbit/spark/VowpalWabbitPerformanceStatistics");
  CHECK_JNI_EXCEPTION(nullptr);

  jmethodID ctor = env->GetMethodID(clazz, "<init>", "(JDDDFFJ)V");
  CHECK_JNI_EXCEPTION(nullptr);

  return env->NewObject(clazz, ctor, numberOfExamplesPerPass, weightedExampleSum, weightedLabelSum, averageLoss,
      bestConstant, bestConstantLoss, totalNumberOfFeatures);
}

JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitNative_endPass(JNIEnv* env, jobject vwObj)
{
  auto* all = reinterpret_cast<VW::workspace*>(get_native_pointer(env, vwObj));

  try
  {
    // note: this code duplication seems bound for trouble
    // from parse_dispatch_loop.h:26
    // from learner.cc:41
    reset_source(*all, all->num_bits);
    all->do_reset_source = false;
    all->passes_complete++;

    all->current_pass++;
    all->l->end_pass();
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitNative_finish(JNIEnv* env, jobject vwObj)
{
  auto* all = reinterpret_cast<VW::workspace*>(get_native_pointer(env, vwObj));

  try
  {
    VW::sync_stats(*all);
    VW::finish(*all);
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT jint JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitNative_hash(
    JNIEnv* env, jclass, jbyteArray data, jint offset, jint len, jint seed)
{
  CriticalArrayGuard dataGuard(env, data);
  const char* values0 = (const char*)dataGuard.data();

  return (jint)VW::uniform_hash(values0 + offset, len, seed);
}

// VW Example
#define INIT_VARS                                                                                      \
  auto exWrapper = reinterpret_cast<VowpalWabbitExampleWrapper*>(get_native_pointer(env, exampleObj)); \
  VW::workspace* all = exWrapper->_all;                                                                \
  example* ex = exWrapper->_example;

JNIEXPORT jlong JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_initialize(
    JNIEnv* env, jclass, jlong vwPtr, jboolean isEmpty)
{
  auto* all = reinterpret_cast<VW::workspace*>(vwPtr);

  try
  {
    example* ex = VW::alloc_examples(1);
    ex->interactions = &all->interactions;
    ex->extent_interactions = &all->extent_interactions;

    if (isEmpty)
    {
      char empty = '\0';
      VW::read_line(*all, ex, &empty);
    }
    else
      all->example_parser->lbl_parser.default_label(ex->l);

    return reinterpret_cast<jlong>(new VowpalWabbitExampleWrapper(all, ex));
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
    return 0;
  }
}

JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_finish(JNIEnv* env, jobject exampleObj)
{
  INIT_VARS

  try
  {
    VW::dealloc_examples(ex, 1);
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_clear(JNIEnv* env, jobject exampleObj)
{
  INIT_VARS

  try
  {
    VW::empty_example(*all, *ex);
    all->example_parser->lbl_parser.default_label(ex->l);
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
  }
}

void addNamespaceIfNotExists(VW::workspace* all, example* ex, char ns)
{
  if (std::find(ex->indices.begin(), ex->indices.end(), ns) == ex->indices.end()) { ex->indices.push_back(ns); }
}

JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_addToNamespaceDense(
    JNIEnv* env, jobject exampleObj, jchar ns, jint weight_index_base, jdoubleArray values)
{
  INIT_VARS

  try
  {
    addNamespaceIfNotExists(all, ex, ns);

    auto features = ex->feature_space.data() + ns;

    CriticalArrayGuard valuesGuard(env, values);
    double* values0 = (double*)valuesGuard.data();

    int size = env->GetArrayLength(values);
    int mask = (1 << all->num_bits) - 1;

    // pre-allocate
    features->values.reserve(features->values.capacity() + size);
    features->indices.reserve(features->indices.capacity() + size);

    double* values_itr = values0;
    double* values_end = values0 + size;
    for (; values_itr != values_end; ++values_itr, ++weight_index_base)
    {
      float x = *values_itr;
      if (x != 0)
      {
        features->values.push_back_unchecked(x);
        features->indices.push_back_unchecked(weight_index_base & mask);
      }
    }
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_addToNamespaceSparse(
    JNIEnv* env, jobject exampleObj, jchar ns, jintArray indices, jdoubleArray values)
{
  INIT_VARS

  try
  {
    addNamespaceIfNotExists(all, ex, ns);

    auto features = ex->feature_space.data() + ns;

    CriticalArrayGuard indicesGuard(env, indices);
    int* indices0 = (int*)indicesGuard.data();

    CriticalArrayGuard valuesGuard(env, values);
    double* values0 = (double*)valuesGuard.data();

    int size = env->GetArrayLength(indices);
    int mask = (1 << all->num_bits) - 1;

    // pre-allocate
    features->values.reserve(features->values.capacity() + size);
    features->indices.reserve(features->indices.capacity() + size);

    int* indices_itr = indices0;
    int* indices_end = indices0 + size;
    double* values_itr = values0;
    for (; indices_itr != indices_end; ++indices_itr, ++values_itr)
    {
      float x = *values_itr;
      if (x != 0)
      {
        features->values.push_back_unchecked(x);
        features->indices.push_back_unchecked(*indices_itr & mask);
      }
    }
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_setLabel(
    JNIEnv* env, jobject exampleObj, jfloat weight, jfloat label)
{
  INIT_VARS

  try
  {
    label_data* ld = &ex->l.simple;
    ld->label = label;
    auto& red_fts = ex->_reduction_features.template get<simple_label_reduction_features>();
    red_fts.weight = weight;

    VW::count_label(*all->sd, ld->label);
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_setDefaultLabel(JNIEnv* env, jobject exampleObj)
{
  INIT_VARS

  try
  {
    all->example_parser->lbl_parser.default_label(ex->l);
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_setMulticlassLabel(
    JNIEnv* env, jobject exampleObj, jfloat weight, jint label)
{
  INIT_VARS

  try
  {
    MULTICLASS::label_t* ld = &ex->l.multi;

    ld->label = label;
    ld->weight = weight;
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_setCostSensitiveLabels(
    JNIEnv* env, jobject exampleObj, jfloatArray costs, jintArray classes)
{
  INIT_VARS

  try
  {
    COST_SENSITIVE::label* ld = &ex->l.cs;

    int sizeCosts = env->GetArrayLength(costs);
    int sizeClasses = env->GetArrayLength(classes);

    if (sizeCosts != sizeClasses)
    {
      env->ThrowNew(env->FindClass("java/lang/IllegalArgumentException"), "costs and classes length must match");
      return;
    }

    CriticalArrayGuard costsGuard(env, costs);
    float* costs0 = (float*)costsGuard.data();

    CriticalArrayGuard classesGuard(env, classes);
    int* classes0 = (int*)classesGuard.data();

    // loop over weights/labels
    for (int i = 0; i < sizeCosts; i++)
    {
      COST_SENSITIVE::wclass w;
      w.x = costs0[i];
      w.class_index = classes0[i];

      ld->costs.push_back(w);
    }
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_setMultiLabels(
    JNIEnv* env, jobject exampleObj, jintArray classes)
{
  INIT_VARS

  try
  {
    MULTILABEL::labels* ld = &ex->l.multilabels;

    CriticalArrayGuard classesGuard(env, classes);
    int* classes0 = (int*)classesGuard.data();

    int size = env->GetArrayLength(classes);

    for (int i = 0; i < size; i++) ld->label_v.push_back(classes0[i]);
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_setContextualBanditContinuousLabel(
    JNIEnv* env, jobject exampleObj, jfloatArray actions, jfloatArray costs, jfloatArray pdfValues)
{
  INIT_VARS

  try
  {
    int sizeActions = env->GetArrayLength(actions);
    int sizeCosts = env->GetArrayLength(costs);
    int sizePdfValues = env->GetArrayLength(pdfValues);

    if (sizeActions != sizeCosts || sizeCosts != sizePdfValues)
    {
      env->ThrowNew(
          env->FindClass("java/lang/IllegalArgumentException"), "actions, costs and pdfValues length must match");
      return;
    }

    VW::cb_continuous::continuous_label* ld = &ex->l.cb_cont;

    CriticalArrayGuard actionsGuard(env, actions);
    float* actions0 = (float*)actionsGuard.data();

    CriticalArrayGuard costsGuard(env, costs);
    float* costs0 = (float*)costsGuard.data();

    CriticalArrayGuard pdfValuesGuard(env, pdfValues);
    float* pdfValues0 = (float*)pdfValuesGuard.data();

    for (int i = 0; i < sizeActions; i++)
    {
      VW::cb_continuous::continuous_label_elm elm;
      elm.action = actions0[i];
      elm.cost = costs0[i];
      elm.pdf_value = pdfValues0[i];
      ld->costs.push_back(elm);
    }
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_setContextualBanditLabel(
    JNIEnv* env, jobject exampleObj, jint action, jdouble cost, jdouble probability)
{
  INIT_VARS

  try
  {
    CB::label* ld = &ex->l.cb;
    CB::cb_class f;

    f.action = (uint32_t)action;
    f.cost = (float)cost;
    f.probability = (float)probability;

    ld->costs.push_back(f);
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_setSharedLabel(JNIEnv* env, jobject exampleObj)
{
  INIT_VARS

  try
  {
    // https://github.com/VowpalWabbit/vowpal_wabbit/blob/master/vowpalwabbit/parse_example_json.h#L437
    CB::label* ld = &ex->l.cb;
    CB::cb_class f;

    f.partial_prediction = 0.;
    f.action = (uint32_t)VW::uniform_hash("shared", 6 /*length of string*/, 0);
    f.cost = FLT_MAX;
    f.probability = -1.f;

    ld->costs.push_back(f);
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_setSlatesSharedLabel(
    JNIEnv* env, jobject exampleObj, jfloat cost)
{
  INIT_VARS

  try
  {
    auto* ld = &ex->l.slates;
    ld->reset_to_default();
    ld->type = VW::slates::example_type::shared;
    ld->cost = cost;
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_setSlatesActionLabel(
    JNIEnv* env, jobject exampleObj, jint slot_id)
{
  INIT_VARS

  try
  {
    auto* ld = &ex->l.slates;
    ld->reset_to_default();
    ld->type = VW::slates::example_type::action;
    ld->slot_id = slot_id;
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_setSlatesSlotLabel(
    JNIEnv* env, jobject exampleObj, jintArray actions, jfloatArray probs)
{
  INIT_VARS

  try
  {
    int sizeActions = env->GetArrayLength(actions);
    int sizeProbs = env->GetArrayLength(probs);

    if (sizeActions != sizeProbs)
    {
      env->ThrowNew(env->FindClass("java/lang/IllegalArgumentException"), "actions and probs length must match");
      return;
    }

    auto* ld = &ex->l.slates;
    ld->reset_to_default();
    ld->type = VW::slates::example_type::slot;

    CriticalArrayGuard actionsGuard(env, actions);
    float* actions0 = (float*)actionsGuard.data();

    CriticalArrayGuard probsGuard(env, probs);
    float* probs0 = (float*)probsGuard.data();

    for (int i = 0; i < sizeActions; i++)
    {
      ACTION_SCORE::action_score as;
      as.action = actions0[i];
      as.score = probs0[i];
      ld->probabilities.push_back(as);
    }
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT jobject JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_getPrediction(JNIEnv* env, jobject exampleObj)
{
  INIT_VARS

  return getJavaPrediction(env, all, ex);
}

JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_learn(JNIEnv* env, jobject exampleObj)
{
  INIT_VARS

  try
  {
    VW::setup_example(*all, ex);

    all->learn(*ex);

    // as this is not a ring-based example it is not free'd
    VW::LEARNER::as_singleline(all->l)->finish_example(*all, *ex);
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT jobject JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_predict(JNIEnv* env, jobject exampleObj)
{
  INIT_VARS

  try
  {
    VW::setup_example(*all, ex);

    all->predict(*ex);

    // as this is not a ring-based example it is not free'd
    VW::LEARNER::as_singleline(all->l)->finish_example(*all, *ex);

    return getJavaPrediction(env, all, ex);
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
    return nullptr;
  }
}

JNIEXPORT jstring JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_toString(JNIEnv* env, jobject exampleObj)
{
  INIT_VARS

  try
  {
    std::ostringstream ostr;

    ostr << "VowpalWabbitExample(label=";
    auto lp = all->example_parser->lbl_parser;

    if (!memcmp(&lp, &simple_label_parser, sizeof(lp)))
    {
      label_data* ld = &ex->l.simple;
      const auto& red_fts = ex->_reduction_features.template get<simple_label_reduction_features>();
      ostr << "simple " << ld->label << ":" << red_fts.weight << ":" << red_fts.initial;
    }
    else if (!memcmp(&lp, &CB::cb_label, sizeof(lp)))
    {
      CB::label* ld = &ex->l.cb;
      ostr << "CB " << ld->costs.size();

      if (ld->costs.size() > 0)
      {
        ostr << " ";

        CB::cb_class& f = ld->costs[0];

        // Ignore checking if f.action == VW::uniform_hash("shared")
        if (f.partial_prediction == 0 && f.cost == FLT_MAX && f.probability == -1.f)
          ostr << "shared";
        else
          ostr << f.action << ":" << f.cost << ":" << f.probability;
      }
    }
    else
    {
      ostr << "unsupported label";
    }

    ostr << ";";
    for (auto& ns : ex->indices)
    {
      if (ns == 0)
        ostr << "NULL:0,";
      else
      {
        if ((ns >= 'a' && ns <= 'z') || (ns >= 'A' && ns <= 'Z')) ostr << "'" << (char)ns << "':";

        ostr << (int)ns << ",";
      }

      for (auto& f : ex->feature_space[ns])
      {
        auto idx = f.index();
        ostr << (idx & all->weights.mask()) << "/" << idx << ":" << f.value() << ", ";
      }
    }

    ostr << ")";

    return env->NewStringUTF(ostr.str().c_str());
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
    return nullptr;
  }
}

// re-use prediction conversation methods
jobject multilabel_predictor(example* vec, JNIEnv* env);
jfloatArray scalars_predictor(example* vec, JNIEnv* env);
jobject action_scores_prediction(example* vec, JNIEnv* env);
jobject action_probs_prediction(example* vec, JNIEnv* env);

jobject getJavaPrediction(JNIEnv* env, VW::workspace* all, example* ex)
{
  jclass predClass;
  jmethodID ctr;
  switch (all->l->get_output_prediction_type())
  {
    case VW::prediction_type_t::scalar:
      predClass = env->FindClass("org/vowpalwabbit/spark/prediction/ScalarPrediction");
      CHECK_JNI_EXCEPTION(nullptr);

      ctr = env->GetMethodID(predClass, "<init>", "(FF)V");
      CHECK_JNI_EXCEPTION(nullptr);

      return env->NewObject(predClass, ctr, VW::get_prediction(ex), ex->confidence);

    case VW::prediction_type_t::prob:
      predClass = env->FindClass("java/lang/Float");
      CHECK_JNI_EXCEPTION(nullptr);

      ctr = env->GetMethodID(predClass, "<init>", "(F)V");
      CHECK_JNI_EXCEPTION(nullptr);

      return env->NewObject(predClass, ctr, ex->pred.prob);

    case VW::prediction_type_t::multiclass:
      predClass = env->FindClass("java/lang/Integer");
      CHECK_JNI_EXCEPTION(nullptr);

      ctr = env->GetMethodID(predClass, "<init>", "(I)V");
      CHECK_JNI_EXCEPTION(nullptr);

      return env->NewObject(predClass, ctr, ex->pred.multiclass);

    case VW::prediction_type_t::scalars:
      return scalars_predictor(ex, env);

    case VW::prediction_type_t::action_probs:
      return action_probs_prediction(ex, env);

    case VW::prediction_type_t::action_scores:
      return action_scores_prediction(ex, env);

    case VW::prediction_type_t::multilabels:
      return multilabel_predictor(ex, env);

    default:
    {
      std::ostringstream ostr;
      ostr << "prediction type '" << VW::to_string(all->l->get_output_prediction_type()) << "' is not supported";

      env->ThrowNew(env->FindClass("java/lang/UnsupportedOperationException"), ostr.str().c_str());
      return nullptr;
    }
  }
}
