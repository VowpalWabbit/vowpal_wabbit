#pragma once
#include <string.h>
#include <unordered_map>
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
class sparse_weight_parameters;
typedef std::unordered_map<size_t, weight*> weight_map;

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
	
	template<void(*T)(iterator&, uint64_t)> //for random initialization of weights (with stride) 
	inline void set_default()
	{  uint32_t stride = 1 << _stride_shift;
	   iterator iter = begin();
	   for (size_t i = 0; iter != end(); ++iter, i += stride)
			T(iter, i);
	}

	template<void(*T)(iterator&, uint64_t, uint32_t)> //for random initialization of the entire weight_vector 
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

template <typename T>
class sparse_weights_iterator
{
private:
	size_t _index;
	//T* _current;
	uint32_t _stride;
	sparse_weight_parameters& _map;

public:
	typedef std::forward_iterator_tag iterator_category;
	typedef T value_type;
	typedef ptrdiff_t difference_type;
	typedef  T* pointer;
	typedef  T& reference;

	typedef weights_iterator_iterator<T> w_iter;

	sparse_weights_iterator(sparse_weight_parameters& map, size_t index, uint32_t stride)
		: _map(map) , _index(index), _stride(stride)
	{ }

	sparse_weights_iterator& operator=(const sparse_weights_iterator& other)
	{
		_index = other._index;
		_stride = other._stride;
		_map = other._map;
		return *this;

	}
	T& operator*() { 
		return _map.iter_helper(_index);
	} 

	sparse_weights_iterator& operator++()
	{  
		_index++;
		return *this;
	}

	sparse_weights_iterator operator+(size_t ind) { return sparse_weights_iterator(_map, _index + ind, _stride); }

	sparse_weights_iterator& operator+=(size_t ind)
	{
		_index += ind;
		return *this;
	}

	bool operator==(const sparse_weights_iterator& rhs) const { return _index == rhs._index; }
	bool operator!=(const sparse_weights_iterator& rhs) const { return _index != rhs._index; }

	//to iterate within a bucket
	w_iter begin() { return w_iter(&_map.iter_helper(_index));}//&_map[_index*_stride]); }
	w_iter end(size_t offset) { return w_iter(&_map.iter_helper(_index) + offset); }
};

struct proxy_object{
	sparse_weight_parameters& _map;
	size_t _i;
	proxy_object(sparse_weight_parameters& map, size_t i) : _map(map), _i(i){};
	proxy_object &operator=(weight &value) {
		_map.write(_i, value);
	}
	operator weight() {
		_map.read(_i);
	}
};
class sparse_weight_parameters
{
private:
	weight_map _map;
	uint64_t _weight_mask;  // (stride*(1 << num_bits) -1)
	uint32_t _stride_shift;
	bool _seeded; // whether the instance is sharing model state with others
	bool _delete = false;

public:
	typedef sparse_weights_iterator<weight> iterator;
	typedef sparse_weights_iterator<const weight> const_iterator;
	void(*fun_1)(iterator&) = nullptr;
	void(*fun_2)(iterator&, size_t) = nullptr;
	void(*fun_3)(iterator&, size_t, uint32_t) = nullptr;
	std::function<void(iterator&, size_t)> fun_4 = nullptr;

	sparse_weight_parameters(size_t length, uint32_t stride_shift = 0)
		: _map(),//_begin(calloc_mergable_or_throw<weight>(length << stride_shift)),
		_weight_mask((length << stride_shift) - 1),
		_stride_shift(stride_shift),
		_seeded(false)
	{
		_map.insert(std::make_pair(0, calloc_mergable_or_throw<weight>(_stride_shift)));
	}

	sparse_weight_parameters()
		: _map(), _weight_mask(0), _stride_shift(0), _seeded(false)
	{}

	bool not_null() { return (_weight_mask > 0 && !_map.empty()); }

	sparse_weight_parameters(const sparse_weight_parameters &other) { shallow_copy(other); }
	sparse_weight_parameters(sparse_weight_parameters &&) = delete;

	weight* first() { throw 1; } //TODO: not currently supported in sparse

	//iterator with stride 
	iterator begin() { return iterator(*this, 0, (1 << _stride_shift)); }
	iterator end() { return iterator(*this, _weight_mask + 1, (1 << _stride_shift)); }

	iterator change_begin() { return iterator(*this, 0, 1); }
	//const iterator
	const_iterator cbegin() { return const_iterator(*this, 0, (1 << _stride_shift)); }
	const_iterator cend() { return const_iterator(*this, _weight_mask + 1, (1 << _stride_shift)); }

	
	inline weight& operator[](size_t i)
	{
		return proxy_object(*this, i);
	}
	weight& read(size_t i){
		size_t index = ceil((i & _weight_mask)/ _stride_shift);
		weight_map::iterator iter = _map.find(index);
		weight_map::iterator end = _map.end();
		if (iter == end) 
		{ 
			iter = _map.find(0);
			memset(iter->second, 0, _stride_shift);
			if (fun_1 != nullptr){
				fun_1(iterator(*this, 0, _stride_shift));
			}
			else if (fun_2 != nullptr){
				fun_2(iterator(*this, 0, _stride_shift), index << _stride_shift);
			}
			else if (fun_3 != nullptr){
				fun_3(iterator(*this, 0, _stride_shift), index << _stride_shift, _stride_shift);
			}
			else if (fun_4 != nullptr){
				fun_4(iterator(*this, 0, _stride_shift), index << _stride_shift);
			}
		}
		 /* Found, i->first is f, i->second is ++-- */
			weight* it = iter->second;
			size_t offset = (i & _weight_mask) % _stride_shift;
			return (&(*it))[offset];
		
	}
	weight& write(size_t i, weight& value){
		size_t index = ceil((i & _weight_mask) / _stride_shift);
		weight_map::iterator iter = _map.find(index);
		weight_map::iterator end = _map.end();
		if (iter == end)
		{

			_map.insert(std::make_pair(index, calloc_mergable_or_throw<weight>(_stride_shift)));
			if (fun_1 != nullptr){
				fun_1(iterator(*this, index, _stride_shift));
			}
			else if (fun_2 != nullptr){
				fun_2(iterator(*this, index, _stride_shift), index << _stride_shift);
			}
			else if (fun_3 != nullptr){
				fun_3(iterator(*this, index, _stride_shift), index << _stride_shift, _stride_shift);
			}
			else if (fun_4 != nullptr){
				fun_4(iterator(*this, index, _stride_shift), index << _stride_shift);
			}
		}
		iter = _map.find(index);
		/* Found, i->first is f, i->second is ++-- */
		weight* it = iter->second;
		size_t offset = (i & _weight_mask) % _stride_shift;
		(&(*it))[offset] = value;

	}
	//definitely need to change this
	inline weight& iter_helper(size_t index)
	{
		weight_map::iterator iter = _map.find(index);
		weight_map::iterator end = _map.end();
		if (iter == end)
		{  _map.insert(std::make_pair(index, calloc_mergable_or_throw<weight>(_stride_shift)));
			if (fun_1 != nullptr){
				fun_1(iterator(*this, index, _stride_shift));
			}
			else if (fun_2 != nullptr){
				fun_2(iterator(*this, index, _stride_shift), index << _stride_shift);
			}
			else if (fun_3 != nullptr){
				fun_3(iterator(*this, index, _stride_shift), index << _stride_shift, _stride_shift);
			}
			else if (fun_4 != nullptr){
				fun_4(iterator(*this, index, _stride_shift), index << _stride_shift);
			}
		}
		iter = _map.find(index);
		/* Found, i->first is f, i->second is ++-- */
		return *(iter->second);

	}
	void shallow_copy(const sparse_weight_parameters& input)
	{
		_map = input._map;
		_weight_mask = input._weight_mask;
		_stride_shift = input._stride_shift;
		_seeded = true;
	}

	template<void(*T)(iterator&)>
	inline void set_default()
	{
		fun_1 = T;
	}

	template<void(*T)(iterator&, size_t)> //for random initialization of weights (with stride) 
	inline void set_default()
	{
		fun_2 = T;
	}

	template<void(*T)(iterator&, size_t, uint32_t)> //for random initialization of the entire weight_vector 
	inline void set_default()
	{
		fun_3 = T;
	}

	template <typename T>
	void set_default(T t)
	{
		fun_4 = t;
	}


	void set_zero(size_t offset)
	{
		for (weight_map::iterator iter = _map.begin(); iter != _map.end(); ++iter){
			(&(*(iter->second)))[offset] = 0;
		}
	}

	uint64_t mask()
	{
		return _weight_mask;
	}

	uint64_t seeded()
	{
		return _seeded;
	}

	uint32_t stride_shift()
	{
		return _stride_shift;
	}

	void stride_shift(uint32_t stride_shift)
	{
		_stride_shift = stride_shift;
	}

#ifndef _WIN32
	void share(size_t length)
	{throw 1; //TODO: add better exceptions
	}
#endif

	~sparse_weight_parameters()
	{if (!_delete && !_seeded)  // don't free weight vector if it is shared with another instance
		{
		 _map.clear();
		 _delete = true;
		}
	}
};




