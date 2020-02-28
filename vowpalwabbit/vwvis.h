#pragma once


#ifdef MS_CONV
#define VW_DEFAULT_VIS
#else
#define VW_DEFAULT_VIS __attribute__((__visibility__("default")))
#endif
