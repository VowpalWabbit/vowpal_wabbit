#pragma once
#include <string.h>

#ifndef _WIN32
#include <sys/mman.h>
#endif

// It appears that on OSX MAP_ANONYMOUS is mapped to MAP_ANON
// https://github.com/leftmike/foment/issues/4
#ifdef __APPLE__
#define MAP_ANONYMOUS MAP_ANON
#endif

typedef float weight;

class weight_parameters;

template <typename T> 
class weights_iterator_iterator
{
private:
	T* _cur;
public:
	weights_iterator_iterator(T* cur)
		: _cur(cur)
	{ }
	
	T& operator*() { return *_cur; }

	weights_iterator_iterator& operator++()
	{
		++_cur;
		return *this;
	}

	weights_iterator_iterator operator+(size_t index) { return weights_iterator_iterator(_cur + index); }

	weights_iterator_iterator& operator+=(size_t index)
	{
		_cur += index;
		return *this;
	}

	bool operator==(const weights_iterator_iterator& rhs) const { return _cur == rhs._cur; }
	bool operator!=(const weights_iterator_iterator& rhs) const { return _cur != rhs._cur; }

};

template <typename T>
class weights_iterator
{
private:
	T* _current;
	uint32_t _stride;

public:
	typedef std::forward_iterator_tag iterator_category;
	typedef T value_type;
	typedef ptrdiff_t difference_type;
	typedef  T* pointer;
	typedef  T& reference;

	typedef weights_iterator_iterator<T> w_iter;
	
	weights_iterator(T* current, uint32_t stride)
		: _current(current), _stride(stride)
	{ }

	T& operator*() { return *_current; }

	weights_iterator& operator++()
	{
		_current += _stride;
		return *this;
	}

	weights_iterator operator+(size_t index) { return weights_iterator(_current + (index*_stride), _stride); }

	weights_iterator& operator+=(size_t index)
	{  _current += (index*_stride);
	   return *this;
	}

	bool operator==(const weights_iterator& rhs) const { return _current == rhs._current; }
	bool operator!=(const weights_iterator& rhs) const { return _current != rhs._current; }

	//to iterate within a bucket
	w_iter begin() { return w_iter(_current); }
	w_iter end(size_t offset) { return w_iter(_current + offset); }
};

class weight_parameters 
{
private:
	weight* _begin;
	uint64_t _weight_mask;  // (stride*(1 << num_bits) -1)
	uint32_t _stride_shift;
	bool _seeded; // whether the instance is sharing model state with others

public:
	typedef weights_iterator<weight> iterator;
	typedef weights_iterator<const weight> const_iterator;

	weight_parameters(size_t length, uint32_t stride_shift=0)
		: _begin(calloc_mergable_or_throw<weight>(length << stride_shift)),
		_weight_mask((length << stride_shift) - 1),	
		_stride_shift(stride_shift),
		_seeded(false)
		{ }

 weight_parameters()
	 : _begin(nullptr), _weight_mask(0), _stride_shift(0), _seeded(false)
	  {}
	
	bool not_null() { return (_weight_mask > 0 && _begin != nullptr);}

	weight_parameters(const weight_parameters &other) { shallow_copy(other); }
	weight_parameters(weight_parameters &&) = delete;

	weight* first() { return _begin; } //TODO: Temporary fix for allreduce.
	
	//iterator with stride 
	iterator begin() { return iterator(_begin, (1<<_stride_shift)); }
	iterator end() { return iterator(_begin + _weight_mask + 1, (1 << _stride_shift)); }

	iterator change_begin() { return iterator(_begin, 1); }
	//const iterator
	const_iterator cbegin() { return const_iterator(_begin, (1 << _stride_shift)); }
	const_iterator cend() { return const_iterator(_begin + _weight_mask + 1, (1 << _stride_shift)); }

	inline weight& operator[](size_t i) const { return _begin[i & _weight_mask]; }
	void shallow_copy(const weight_parameters& input)
	{ _begin = input._begin;
	  _weight_mask = input._weight_mask;
	  _stride_shift = input._stride_shift;
	  _seeded = true;
	}

	template<void(*T)(iterator&)>
    inline void set_default()
	  {
	    for (iterator iter = begin(); iter != end(); ++iter)
	      T(iter);
	  }
	
	template<void(*T)(iterator&, size_t)> //for random initialization of weights (with stride) 
    inline void set_default()
	{  uint32_t stride = 1 << _stride_shift;
	   iterator iter = begin();
	   for (size_t i = 0; iter != end(); ++iter, i += stride)
			T(iter, i);
	}

	template<void(*T)(iterator&, size_t, uint32_t)> //for random initialization of the entire weight_vector 
    inline void set_default()
	{ uint32_t stride = 1 << _stride_shift;
	iterator iter = begin();
	  for (size_t i = 0; iter != end(); ++iter, i += stride)
			T(iter, i, stride);
	}

	template <typename T>
    void set_default(T t)
	{ uint32_t stride = 1 << _stride_shift;
	  iterator iter = begin();
	   for (size_t i = 0; iter != end(); ++iter, i+= stride)
			t(iter, i); 
	}
	

	void set_zero(size_t offset)
	{
		for (iterator iter = begin(); iter != end(); ++iter)
			(&(*iter))[offset] = 0;
	}
	
	uint64_t mask()
	{ return _weight_mask;
	}

	uint64_t seeded()
	{  return _seeded;
	}

	uint32_t stride_shift()
	{ return _stride_shift;		
	}		
	
	void stride_shift(uint32_t stride_shift)		
	{ _stride_shift = stride_shift;		
	}
	
	#ifndef _WIN32
	void share(size_t length)
	{
	  float* shared_weights = (float*)mmap(0, (length << _stride_shift) * sizeof(float),
			                  PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
          size_t float_count = length << _stride_shift;
      	  weight* dest = shared_weights;
	  memcpy(dest, _begin, float_count*sizeof(float));
      	  free(_begin);
      	  _begin = dest;
	}
	#endif
	
	~weight_parameters()
	{  if (_begin != nullptr && !_seeded)  // don't free weight vector if it is shared with another instance
	   {  free(_begin);
	      _begin = nullptr;
	   }
	}
};


