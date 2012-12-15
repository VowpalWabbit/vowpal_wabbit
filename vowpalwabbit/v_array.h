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
  void resize(size_t length)
    {
      if ((size_t)(end_array-begin) != length)
	{
	  size_t old_len = end-begin;
	  begin = (T *)realloc(begin, sizeof(T) * length);
	  if ((begin == NULL) && ((sizeof(T)*length) > 0)) {
	    std::cerr << "realloc of " << length << " failed in reserve().  out of memory?" << std::endl;
	    exit(-1);
	  }
	  end = begin+old_len;
	  end_array = begin + length;
	}
    }

  void erase() 
  { if (++erase_count & erase_point && end_array != end)
      {
	size_t new_len = end-begin;
	begin = (T *)realloc(begin, sizeof(T)*new_len);
	end_array = begin + new_len;
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
      {
	size_t old_length = end_array - begin;
	size_t new_length = 2 * old_length + 3;
	//      size_t new_length = old_length + 1;
	begin = (T *)realloc(begin,sizeof(T) * new_length);
	end = begin + old_length;
	end_array = begin + new_length;
      }
    *(end++) = new_ele;
  }
};


#ifdef _WIN32
#undef max
#undef min
inline size_t max(size_t a, size_t b)
{ if ( a < b) return b; else return a;
}
inline size_t min(size_t a, size_t b)
{ if ( a < b) return a; else return b;
}
#else
inline size_t max(size_t a, size_t b)
{ if ( a < b) return b; else return a;
}
inline size_t min(size_t a, size_t b)
{ if ( a < b) return a; else return b;
}
#endif

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
    {
      size_t length = v.end - v.begin;
      size_t new_length = max(2 * (size_t)(v.end_array - v.begin) + 3, 
			      v.end - v.begin + num);
      v.begin = (T *)realloc(v.begin,sizeof(T) * new_length);
      v.end = v.begin + length;
      v.end_array = v.begin + new_length;
    }
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
