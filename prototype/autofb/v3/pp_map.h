/*
 * Created by William Swanson in 2012.
 *
 * I, William Swanson, dedicate this work to the public domain.
 * I waive all rights to the work worldwide under copyright law,
 * including all related and neighboring rights,
 * to the extent allowed by law.
 *
 * You can copy, modify, distribute and perform the work,
 * even for commercial purposes, all without asking permission.
 */

#pragma once

#define PP_EVAL0(...) __VA_ARGS__
#define PP_EVAL1(...) PP_EVAL0(PP_EVAL0(PP_EVAL0(__VA_ARGS__)))
#define PP_EVAL2(...) PP_EVAL1(PP_EVAL1(PP_EVAL1(__VA_ARGS__)))
#define PP_EVAL3(...) PP_EVAL2(PP_EVAL2(PP_EVAL2(__VA_ARGS__)))
#define PP_EVAL4(...) PP_EVAL3(PP_EVAL3(PP_EVAL3(__VA_ARGS__)))
#define PP_EVAL(...)  PP_EVAL4(PP_EVAL4(PP_EVAL4(__VA_ARGS__)))

#define PP_MAP_END(...)
#define PP_MAP_OUT
#define PP_MAP_COMMA ,

#define PP_MAP_GET_END2() 0, PP_MAP_END
#define PP_MAP_GET_END1(...) PP_MAP_GET_END2
#define PP_MAP_GET_END(...) PP_MAP_GET_END1
#define PP_MAP_NEXT0(test, next, ...) next PP_MAP_OUT
#define PP_MAP_NEXT1(test, next) PP_MAP_NEXT0(test, next, 0)
#define PP_MAP_NEXT(test, next)  PP_MAP_NEXT1(PP_MAP_GET_END test, next)

#define PP_MAP0(f, x, peek, ...) f(x) PP_MAP_NEXT(peek, PP_MAP1)(f, peek, __VA_ARGS__)
#define PP_MAP1(f, x, peek, ...) f(x) PP_MAP_NEXT(peek, PP_MAP0)(f, peek, __VA_ARGS__)

#define PP_MAP_LIST_NEXT1(test, next) PP_MAP_NEXT0(test, PP_MAP_COMMA next, 0)
#define PP_MAP_LIST_NEXT(test, next)  PP_MAP_LIST_NEXT1(PP_MAP_GET_END test, next)

#define PP_MAP_LIST0(f, x, peek, ...) f(x) PP_MAP_LIST_NEXT(peek, PP_MAP_LIST1)(f, peek, __VA_ARGS__)
#define PP_MAP_LIST1(f, x, peek, ...) f(x) PP_MAP_LIST_NEXT(peek, PP_MAP_LIST0)(f, peek, __VA_ARGS__)

/**
 * Applies the function macro `f` to each of the remaining parameters.
 */
#define PP_MAP(f, ...) PP_EVAL(PP_MAP1(f, __VA_ARGS__, ()()(), ()()(), ()()(), 0))

/**
 * Applies the function macro `f` to each of the remaining parameters and
 * inserts commas between the results.
 */
#define PP_MAP_LIST(f, ...) PP_EVAL(PP_MAP_LIST1(f, __VA_ARGS__, ()()(), ()()(), ()()(), 0))