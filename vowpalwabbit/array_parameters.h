#pragma once
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>

using namespace std;
typedef float weight;

class weight_vector;

class weight_walker
{
private:
	weight* _cur;
public:
	weight_walker(weight* cur)
		: _cur(cur)
	{ }
	
	weight& operator*() { return *_cur; }

	weight_walker& operator++()
	{
		_cur++;
		return *this;
	}

	template<typename T>
	weight_walker operator+(T index) { return weight_walker(_cur + index); }

	template<typename T>
	weight_walker& operator+=(T index)
	{
		_current += index;
		return *this;
	}

	weight_walker& operator=(const weight_walker& other)
	{
		_cur = other._cur;
		return *this;
	}

	bool operator==(const weight_walker& rhs) { return _cur == rhs._cur; }
	bool operator!=(const weight_walker& rhs) { return _cur != rhs._cur; }

	~weight_walker(){
		if (_cur != nullptr)
			free(_cur);
	}
};
class weights_iterator
{
private:
	weight* _current;
	uint32_t _stride_shift;

public:
	typedef weight_walker w_iter;

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

	~weights_iterator(){
		if (_current != nullptr)
			free(_current);
	}
};

class weight_vector //different name? (previously array_parameters)
{
private:
	weight* _begin;
	uint64_t _weight_mask;  // (stride*(1 << num_bits) -1)
	uint32_t _stride_shift = 0;
	size_t _size;

public:
	typedef weights_iterator iterator;

	weight_vector()
		: _begin(nullptr), _weight_mask((uint64_t)LONG_MAX), _size(0)
	{ }

	weight_vector(size_t length)
		: _begin(new weight[length << _stride_shift]()), _weight_mask((length << _stride_shift) - 1), _size(length << _stride_shift)
	{ }

	inline weight*& first() { return _begin; } //TODO: Temporary fix for lines like (&w - all.reg.weight_vector). Needs to change for sparse.
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

	uint64_t getMask()
	{
		return _weight_mask;
	}

	uint32_t getStride()
	{
		return _stride_shift;
	}
	//TODO: needs to be removed.
	void setStride(uint32_t stride)
	{
		_stride_shift = stride;
	}

	~weight_vector(){
		if (_begin != nullptr)
			free(_begin);
	}
	friend weights_iterator;
};


