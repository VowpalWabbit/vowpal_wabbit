#ifdef _WIN32
#define NOMINMAX
#include <immintrin.h>

// typedef float(*sum_of_squares_func)(float*, float*);
//
// sum_of_squares_func get_sum_of_squares();

float sum_of_squares(float* begin, float* end);

// float sum_of_squares_avx(float* begin, float* end);
// float sum_of_squares_avx2(float* begin, float* end);

#endif
