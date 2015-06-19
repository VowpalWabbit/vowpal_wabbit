#include <stdexcept>
#include <sstream>

class vw_exception : public std::exception
{
	std::stringstream message;

public:
	vw_exception();
	vw_exception(vw_exception&);
	
	template<typename T>
	vw_exception(T msg)
	{
		message << msg;
	}

	virtual const char * what() const;
	
	template<typename T>
	friend vw_exception& operator<<(vw_exception& os, T msg)
	{
		os.message << msg;
		return os;
	}
};

// for use with file and line numbers
#define THROW(args) { vw_exception __err; __err.message << args; throw __err; }
