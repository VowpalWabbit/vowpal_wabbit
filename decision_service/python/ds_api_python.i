%module(threads="1", directors="1") decision_service

// make sure this function can block on a thread and python threads can continue running

%nothread;
%thread DecisionServiceConfiguration::Download; 
%thread DecisionServiceListener::log;

#define ARRAYS_OPTIMIZED
#define DISABLE_NAMESPACE

// generate documentation
// TODO: https://github.com/m7thon/doxy2swig
%feature("autodoc", "2");

%include "../ds_api.i"

%include <pybuffer.i>
%pybuffer_binary(unsigned char* model, size_t len)

namespace std
{
	%template(IntVector) vector<int>;
	%template(FloatVector) vector<float>;
}

/* useful for debugging
%feature("director:except") {
  if ($error != NULL) {
    PyObject *exc, *val, *tb;
    PyErr_Fetch(&exc, &val, &tb);
    PyErr_NormalizeException(&exc, &val, &tb);
    std::string err_msg("In method '$symname': ");

    PyObject* exc_str = PyObject_GetAttrString(exc, "__name__");
    err_msg += PyUnicode_AsUTF8(exc_str);
    Py_XDECREF(exc_str);
    
    if (val != NULL)
    {
      PyObject* val_str = PyObject_Str(val);
      err_msg += ": ";
      err_msg += PyUnicode_AsUTF8(val_str);
      Py_XDECREF(val_str);
    }

    Py_XDECREF(exc);
    Py_XDECREF(val);
    Py_XDECREF(tb);
    
    Swig::DirectorMethodException::raise(err_msg.c_str());
  }
}
*/

%ignore Array<int>;
%ignore DecisionServiceClient::rank_cstyle;
%ignore DecisionServiceClient::rank_struct;
%ignore DecisionServiceClient::update_model(unsigned char*, size_t, size_t);
%rename(rank) DecisionServiceClient::rank_vector;


%include "ds_api.h"

