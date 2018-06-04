cmake_minimum_required (VERSION 3.8)
project (cs_common LANGUAGES CSharp)

FILE(GLOB_RECURSE src *.cs)
add_library(cs_common ${src})

set_property(TARGET cs_common PROPERTY VS_DOTNET_TARGET_FRAMEWORK_VERSION "v4.5.2")
target_compile_options(cs_common PRIVATE "/langversion:6")

set_property(TARGET cs_common PROPERTY VS_DOTNET_REFERENCES
	"System")

SET_TARGET_PROPERTIES(cs_common PROPERTIES OUTPUT_NAME VowpalWabbit.Common)

