#include <benchmark/benchmark.h>

#include <functional>
#include <memory>

struct Context;
using FnPtrContext = void(Context*, int);
using FnPtrVoid = void(void*, int);
using FnPtrBound = void(int);

struct Context
{
  int value;
};

struct FnData
{
  // Context object is type erased here
  FnPtrVoid* fn;
  void* context;
};

void add_fn(Context* context, int x)
{
  // read value, add, and write back
  context->value += x;
}

#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__) || defined(__CLANG__)
__attribute__((noinline))
#endif
    void add_fn_noinline(Context* context, int x)
{
  // force this version of the function to not be inlined
  // read value, add, and write back
  context->value += x;
}

template <bool noinline>
struct TestObj
{
  std::unique_ptr<Context> context;

  FnData fn_obj;
  std::function<FnPtrContext> std_function;
  std::function<FnPtrBound> std_function_bind;
  std::function<FnPtrBound> std_function_lambda;

  TestObj() : context(new Context{0})
  {
    Context* context_ptr = context.get();

    if (noinline)
    {
      fn_obj.fn = reinterpret_cast<FnPtrVoid*>(add_fn_noinline);
      fn_obj.context = context_ptr;
      std_function = add_fn_noinline;
      std_function_bind = std::bind(add_fn_noinline, context_ptr, std::placeholders::_1);
      std_function_lambda = [context_ptr](int x) { add_fn_noinline(context_ptr, x); };
    }
    else
    {
      fn_obj.fn = reinterpret_cast<FnPtrVoid*>(add_fn);
      fn_obj.context = context_ptr;
      std_function = add_fn;
      std_function_bind = std::bind(add_fn, context_ptr, std::placeholders::_1);
      std_function_lambda = [context_ptr](int x) { add_fn(context_ptr, x); };
    }
  }
};

void function_call_direct(benchmark::State& state)
{
  Context context{0};
  for (auto _ : state)
  {
    add_fn(&context, 1);
    benchmark::ClobberMemory();
  }
}

void function_call_direct_noinline(benchmark::State& state)
{
  Context context{0};
  for (auto _ : state)
  {
    add_fn_noinline(&context, 1);
    benchmark::ClobberMemory();
  }
}

template <bool noinline>
void function_call_pointer(benchmark::State& state)
{
  TestObj<noinline> test;
  for (auto _ : state)
  {
    test.fn_obj.fn(test.fn_obj.context, 1);
    benchmark::ClobberMemory();
  }
}

template <bool noinline>
void function_call_std_function(benchmark::State& state)
{
  TestObj<noinline> test;
  Context* context_ptr = test.context.get();
  for (auto _ : state)
  {
    test.std_function(context_ptr, 1);
    benchmark::ClobberMemory();
  }
}

template <bool noinline>
void function_call_std_function_bind(benchmark::State& state)
{
  TestObj<noinline> test;
  for (auto _ : state)
  {
    test.std_function_bind(1);
    benchmark::ClobberMemory();
  }
}

template <bool noinline>
void function_call_std_function_lambda(benchmark::State& state)
{
  TestObj<noinline> test;
  for (auto _ : state)
  {
    test.std_function_lambda(1);
    benchmark::ClobberMemory();
  }
}

BENCHMARK(function_call_direct);
BENCHMARK(function_call_direct_noinline);
BENCHMARK(function_call_pointer<false>)->Name("function_call_pointer");
BENCHMARK(function_call_pointer<true>)->Name("function_call_pointer_noinline");
BENCHMARK(function_call_std_function<false>)->Name("function_call_std_function");
BENCHMARK(function_call_std_function<true>)->Name("function_call_std_function_noinline");
BENCHMARK(function_call_std_function_bind<false>)->Name("function_call_std_function_bind");
BENCHMARK(function_call_std_function_bind<true>)->Name("function_call_std_function_bind_noinline");
BENCHMARK(function_call_std_function_lambda<false>)->Name("function_call_std_function_lambda");
BENCHMARK(function_call_std_function_lambda<true>)->Name("function_call_std_function_lambda_noinline");

/*
-------------------------------------------------------------------------------------
Benchmark                                           Time             CPU   Iterations
-------------------------------------------------------------------------------------
function_call_direct                            0.331 ns        0.331 ns   1000000000
function_call_direct_noinline                    1.37 ns         1.37 ns    496880302
function_call_pointer                            2.02 ns         2.02 ns    350094350
function_call_pointer_noinline                   2.02 ns         2.02 ns    343273218
function_call_std_function                       2.66 ns         2.66 ns    255542722
function_call_std_function_noinline              2.63 ns         2.63 ns    260247041
function_call_std_function_bind                  2.33 ns         2.33 ns    283716344
function_call_std_function_bind_noinline         2.35 ns         2.35 ns    291974213
function_call_std_function_lambda                2.02 ns         2.02 ns    334876957
function_call_std_function_lambda_noinline       2.75 ns         2.75 ns    249497886
*/
