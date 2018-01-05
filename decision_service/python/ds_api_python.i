%module(threads="1", directors="1") decision_service

// make sure this function can block on a thread and python threads can continue running

%nothread;
%thread DecisionServiceConfiguration::Download; 
%thread DecisionServiceListener::error;
%thread DecisionServiceListener::trace;

#define ARRAYS_OPTIMIZED
#define DISABLE_NAMESPACE

%include "../ds_api.i"

// try how the vector binding looks like... maybe it's ok
// add update_model w/o offset to support bybuffer

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
%rename(rank) DecisionServiceClient::rank_vector;


%include "ds_api.h"

