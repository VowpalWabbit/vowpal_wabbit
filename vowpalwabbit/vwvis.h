#pragma once


#ifdef _WIN32
#define VW_DEFAULT_VIS
#else
#define VW_DEFAULT_VIS __attribute__((__visibility__("default")))
#endif
