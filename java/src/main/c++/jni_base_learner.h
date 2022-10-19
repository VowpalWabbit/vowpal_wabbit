#ifndef VW_BASE_LEARNER_H
#define VW_BASE_LEARNER_H

#include "util.h"
#include "vw/core/global_data.h"
#include "vw/core/learner.h"
#include "vw/core/vw.h"

#include <jni.h>

#include <functional>

example* read_example(JNIEnv* env, jstring example_string, VW::workspace* vwInstance);
example* read_example(const char* example_string, VW::workspace* vwInstance);

// It would appear that after reading posts like
// http://stackoverflow.com/questions/6458612/c0x-proper-way-to-receive-a-lambda-as-parameter-by-reference
// and
// http://stackoverflow.com/questions/3203305/write-a-function-that-accepts-a-lambda-expression-as-argument
// it is more efficient to use another type parameter instead of std::function<T(example*)>
// but more difficult to read.
template <typename T, typename F>
T base_predict(JNIEnv* env, example* ex, bool learn, VW::workspace* vwInstance, const F& predictor, const bool predict)
{
  T result = 0;
  try
  {
    if (learn)
      vwInstance->learn(*ex);
    else
      vwInstance->predict(*ex);

    if (predict) result = predictor(ex, env);

    vwInstance->finish_example(*ex);
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
  }
  return result;
}

template <typename T, typename F>
T base_predict(JNIEnv* env, jstring example_string, jboolean learn, jlong vwPtr, const F& predictor)
{
  VW::workspace* vwInstance = (VW::workspace*)vwPtr;
  example* ex = read_example(env, example_string, vwInstance);
  return base_predict<T>(env, ex, learn, vwInstance, predictor, true);
}

template <typename T, typename F>
T base_predict(JNIEnv* env, jobjectArray example_strings, jboolean learn, jlong vwPtr, const F& predictor)
{
  VW::workspace* vwInstance = (VW::workspace*)vwPtr;
  int example_count = env->GetArrayLength(example_strings);
  multi_ex ex_coll;  // When doing multiline prediction the final result is stored in the FIRST example parsed.
  example* first_example = NULL;
  for (int i = 0; i < example_count; i++)
  {
    jstring example_string = (jstring)(env->GetObjectArrayElement(example_strings, i));
    example* ex = read_example(env, example_string, vwInstance);
    ex_coll.push_back(ex);
    if (i == 0) first_example = ex;
  }
  env->DeleteLocalRef(example_strings);

  try
  {
    if (learn)
      vwInstance->learn(ex_coll);
    else
      vwInstance->predict(ex_coll);
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
  }

  T result = predictor(first_example, env);
  vwInstance->finish_example(ex_coll);
  return result;
}

#endif  // VW_BASE_LEARNER_H
