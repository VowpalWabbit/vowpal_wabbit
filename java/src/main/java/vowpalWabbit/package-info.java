/**
 * A JNI layer for submitting examples to VW and getting predictions back.  It should be noted
 * that at this time VW is NOT thread safe, and therefore neither is the JNI layer.  It should be noted
 * that this was originally written with a bulk interface that was later removed because of benchmarking
 * data found <a href="https://microbenchmarks.appspot.com/runs/817d246a-5f90-478a-bc27-d5912d2ff874#r:scenario.benchmarkSpec.methodName,scenario.benchmarkSpec.parameters.loss,scenario.benchmarkSpec.parameters.mutabilityPolicy,scenario.benchmarkSpec.parameters.nExamples">here</a>.
 *
 * <b>NOTE</b>: When finished using VW predictors, <code>close</code> <b>MUST</b> be called in order
 * to free up the memory on the C side.
 */
package vowpalWabbit;
