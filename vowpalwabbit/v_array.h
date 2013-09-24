/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#ifndef VARRAY_H__
#define VARRAY_H__
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

#ifdef _WIN32
#define __INLINE 
#else
#define __INLINE inline
#endif

const size_t erase_point = ~ ((1 << 10) -1);

template<class T> class v_array{
 public:
  T* begin;
  T* end;
  T* end_array;
  size_t erase_count;

  T last() { return *(end-1);}
  T pop() { return *(--end);}
  bool empty() { return begin == end;}
  void decr() { end--;}
  v_array() { begin= NULL; end = NULL; end_array=NULL; erase_count = 0;}
  T& operator[](size_t i) { return begin[i]; }
  size_t size(){return end-begin;}
  void resize(size_t length, bool zero_everything=false)
    {
      if ((size_t)(end_array-begin) != length)
	{
	  size_t old_len = end-begin;
	  begin = (T *)realloc(begin, sizeof(T) * length);
	  if ((begin == NULL) && ((sizeof(T)*length) > 0)) {
	    std::cerr << "realloc of " << length << " failed in resize().  out of memory?" << std::endl;
	    throw std::exception();
	  }
          if (zero_everything && (old_len < length))
            memset(begin+old_len, 0, (length-old_len)*sizeof(T));
	  end = begin+old_len;
	  end_array = begin + length;
	}
    }

  void erase() 
  { if (++erase_count & erase_point)
      {
	resize(end-begin);
	erase_count = 0;
      }
    end = begin;
  }
  void delete_v()
  {
    if (begin != NULL)
      free(begin);
    begin = end = end_array = NULL;
  }
  void push_back(const T &new_ele)
  {
    if(end == end_array)
      resize(2 * (end_array-begin) + 3);
    *(end++) = new_ele;
  }
};


#ifdef _WIN32
#undef max
#undef min
#endif

inline size_t max(size_t a, size_t b)
{ if ( a < b) return b; else return a;
}
inline size_t min(size_t a, size_t b)
{ if ( a < b) return a; else return b;
}

template<class T> void copy_array(v_array<T>& dst, v_array<T> src)
{
  dst.erase();
  push_many(dst, src.begin, src.size());
}

template<class T> void copy_array(v_array<T>& dst, v_array<T> src, T(*copy_item)(T))
{
  dst.erase();
  for (T*item = src.begin; item != src.end; item++)
    dst.push_back(copy_item(*item));
}

template<class T> void push_many(v_array<T>& v, const T* begin, size_t num)
{
  if(v.end+num >= v.end_array)
    v.resize(max(2 * (size_t)(v.end_array - v.begin) + 3, 
		 v.end - v.begin + num));
  memcpy(v.end, begin, num * sizeof(T));
  v.end += num;
}

template<class T> void calloc_reserve(v_array<T>& v, size_t length)
{
  v.begin = (T *)calloc(length, sizeof(T));
  v.end = v.begin;
  v.end_array = v.begin + length;
}

template<class T> v_array<T> pop(v_array<v_array<T> > &stack)
{
  if (stack.end != stack.begin)
    return *(--stack.end);
  else
    return v_array<T>();
}

#endif  // VARRAY_H__
