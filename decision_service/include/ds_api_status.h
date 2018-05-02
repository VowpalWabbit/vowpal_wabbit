#pragma once

//error codes
#define DS_SUCCESS 0
#define DS_INVALID_ARGUMENT 1


//failure check macro
#define PRECONDITIONS(x, y) do { \
  int retval = (x); \
  if (retval != DS_SUCCESS) { \
    y; \
  } \
} while (0)


namespace decision_service {

	struct api_status {
		int error_code;
		const char* error_msg;
	};

	//helper to check arguments
	static bool is_null_or_empty(const char * arg, api_status* status)
	{
		if (!arg || strlen(arg) == 0) {
			if (status) {
				//update api status if needed
				status->error_code = DS_INVALID_ARGUMENT;
				status->error_msg = "null or empty argument";
			}
			return true;
		}
		return false;
	}
}