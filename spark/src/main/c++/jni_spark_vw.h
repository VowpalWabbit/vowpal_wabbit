#include "jni_spark_vw_generated.h"
#include "vw.h"

// some JNI helper
class StringGuard {
    JNIEnv* _env;
    jstring _source;
    const char* _cstr;
public:
    StringGuard(JNIEnv *env, jstring source);
    ~StringGuard();

    const char* c_str();
};  

class VowpalWabbitExampleWrapper { 
public:
    vw* _all;
    example* _example;

    VowpalWabbitExampleWrapper(vw* all, example* example)
        : _all(all), _example(example) 
    { }
};

