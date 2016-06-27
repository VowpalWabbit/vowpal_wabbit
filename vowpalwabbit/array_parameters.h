#pragma once
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>

#ifndef _WIN32
#include <sys/mman.h>
#endif

using namespace std;
typedef float weight;

class weight_vector;

class weights_iterator_iterator
{
private:
	weight* _cur;
public:
	weights_iterator_iterator(weight* cur)
		: _cur(cur)
	{ }
	
	weight& operator*() { return *_cur; }

	weights_iterator_iterator& operator++()
	{
		_cur++;
		return *this;
	}

	template<typename T>
	weights_iterator_iterator operator+(T index) { return weights_iterator_iterator(_cur + index); }

	template<typename T>
	weights_iterator_iterator& operator+=(T index)
	{
		_cur += index;
		return *this;
	}

	weights_iterator_iterator& operator=(const weights_iterator_iterator& other)
	{
		_cur = other._cur;
		return *this;
	}

	bool operator==(const weights_iterator_iterator& rhs) { return _cur == rhs._cur; }
	bool operator!=(const weights_iterator_iterator& rhs) { return _cur != rhs._cur; }

};
class weights_iterator
{
private:
	weight* _current;
	uint32_t _stride_shift;

public:
	typedef weights_iterator_iterator w_iter;

	weights_iterator(weight* current, uint32_t stride_shift)
		: _current(current), _stride_shift(stride_shift)
	{ }

	weight& operator*() { return *_current; }

	weights_iterator& operator++()
	{
		_current += _stride_shift;
		return *this;
	}

	template<typename T>
	weights_iterator operator+(T index) { return weights_iterator(_current + (index*_stride_shift), _stride_shift); }

	template<typename T>
	weights_iterator& operator+=(T index)
	{
		_current += (index*_stride_shift);
		return *this;
	}

	weights_iterator& operator=(const weights_iterator& other)
	{
		_current = other._current;
		_stride_shift = other._stride_shift;
		return *this;
	}

	bool operator==(const weights_iterator& rhs) { return _current == rhs._current; }
	bool operator!=(const weights_iterator& rhs) { return _current != rhs._current; }

	//to iterate within a bucket
	w_iter begin() { return w_iter(_current); }
	w_iter end(size_t offset) { return w_iter(_current + offset); }

};

class weight_vector //different name? (previously array_parameters)
{
private:
	weight* _begin;
	uint64_t _weight_mask;  // (stride*(1 << num_bits) -1)
	uint32_t _stride_shift;
	size_t _size;

public:
	typedef weights_iterator iterator;

	weight_vector()
		: _begin(nullptr), _weight_mask((uint64_t)LONG_MAX), _size(0), _stride_shift(0)
	{ }

	weight_vector(size_t length, uint32_t _stride_shift=0)
		: _begin(calloc_mergable_or_throw<weight>(length << _stride_shift)), _weight_mask((length << _stride_shift) - 1), _size(length << _stride_shift)
	{ }

	inline weight* first() { return _begin; } //TODO: Temporary fix for lines like (&w - all.reg.weight_vector). Needs to change for sparse.
	iterator begin() { return iterator(_begin, 1); }
	iterator end() { return iterator(_begin + _size, 1); }

	//iterator with stride and offset
	iterator begin(size_t offset) { return iterator(_begin + offset, (1<<_stride_shift)); }
	iterator end(size_t offset) { return iterator(_begin + _size + offset, (1 << _stride_shift)); }
	
	bool isNull() const { return _begin == nullptr; }

	//weight_vector(const weight_vector& other)
	//	: _begin(other._begin)
	//{ }

	inline weight& operator[](size_t i) const { return _begin[i & _weight_mask]; }

	uint64_t mask()
	{
		return _weight_mask;
	}

	uint32_t stride()
	{
		return _stride_shift;
	}
	//TODO: needs to be removed.
	void stride(uint32_t stride)
	{
		_stride_shift = stride;
	}

	void share(size_t length)
	{
	  #ifndef _WIN32
	  float* shared_weights = (float*)mmap(0, (length << _stride_shift) * sizeof(float),
			                  PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
          size_t float_count = length << _stride_shift;
      	  weight* dest = shared_weights;
	  memcpy(dest, _begin, float_count*sizeof(float));
      	  free(_begin);
      	  _begin = dest;
     	  #endif
	}
	~weight_vector(){
		if (_begin != nullptr)
			free(_begin);
	}
	friend class weights_iterator;
};


