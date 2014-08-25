typedef unsigned __int64 u64;
typedef unsigned __int32 u32;
typedef unsigned __int16 u16;
typedef unsigned __int8  u8;
typedef unsigned char    byte;

typedef signed __int64 i64;
typedef signed __int32 i32;
typedef signed __int16 i16;
typedef signed __int8  i8;

/*
typedef struct _GUID {
	u32 Data1;
	u16 Data2;
	u16 Data3;
	u8 Data4[8];
} GUID;
*/

//TODO: Create a custom Guid class, use std::atomic internally for now