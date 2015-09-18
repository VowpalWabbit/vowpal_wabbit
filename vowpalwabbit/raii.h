#pragma once

template<typename T>
class Writable;

template<typename T>
class Property
{
private:
	T _value;

public:
	T& operator=(const T& value) = delete;

	inline operator T() const
	{
		return _value;
	}

	friend class Writable<T>;
};

template<typename T>
class VArray
{
private:
	T* begin;
	T* end;
	T* end_array;
	size_t erase_count;

public:
	VArray()
	{
		begin = end = new T[100];
		end_array = end + 100;
	}

	T& operator[](size_t i) { return begin[i]; }

	// loop using std::iterator

	class push_back_state
	{
	private:
		VArray* _array;

	public:
		push_back_state(VArray<T>& varray, const T& value) : _array(&varray)
		{
			// ignore resize
			*(_array->end++) = value;
		}

		~push_back_state()
		{
			_array->end--;
		}
	};

	class pop_state
	{
	private:
		VArray* _array;
		T _value;

	public:
		pop_state(VArray<T>& varray) : _array(&varray)
		{
			// ignore resize
			_value = *(--_array->end);
		}

		~pop_state()
		{
			*(_array->end++) = _value;
		}
	};

	const push_back_state push_back(const T& value)
	{
		return push_back_state(*this, value);
	}

	const pop_state pop()
	{
		return pop_state(*this);
	}
};

void foo()
{
	VArray<int> arr;

	VArray<int>::push_back_state s1(arr, 25);
}


class Example
{
public:
	Property<float> Foo;

	Property<int> Bar;
};

template<typename T>
class Writable
{
private:
	T _old_value;
	T* _target;

public:
	Writable(const Property<T>& prop)
	{
		// copy
		_old_value = prop._value;

		_target = (T*)&prop._value;
	}

	~Writable()
	{
		*_target = _old_value;
	}

	inline T& operator=(const T& value)
	{
		return *_target = value;
	}

	inline operator T() const
	{
		return *_target;
	}
};


int reduction_1(const Example& ex)
{
	VArray<int> x;

	// push_back s1(x, 5);


	Writable<float> foo(ex.Foo);

	//x.Foo = 23;

	foo = 23;

	return ex.Foo;
}
