project (cs_cli)

set(CMAKE_CXX_STANDARD 11)

# TODO: check for -DCMAKE_GENERATOR_PLATFORM=x64

add_library(${PROJECT_NAME} SHARED AssemblyInfo.cpp clr_io.cpp clr_io_memory.cpp vowpalwabbit.cpp vw_base.cpp vw_builder.cpp vw_cbutil.cpp vw_example.cpp vw_exception.cpp vw_model.cpp vw_prediction.cpp)

add_custom_command(TARGET ${PROJECT_NAME}
    PRE_BUILD
    COMMAND nuget install -OutputDirectory ${CMAKE_BINARY_DIR}/packages Newtonsoft.Json
)

set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DOTNET_REFERENCE_Newtonsoft.Json
	"${CMAKE_BINARY_DIR}/packages/Newtonsoft.Json.11.0.2/lib/net45/Newtonsoft.Json.dll")

set_target_properties(${PROJECT_NAME} PROPERTIES VS_GLOBAL_TargetFrameworkVersion "v4.5.2")
#set(CMAKE_VS_PLATFORM_TOOLSET "v4.5.2")

STRING(REPLACE "/EHsc" "/EHa" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
STRING(REPLACE "/RTC1" "" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})

include_directories(${vw_INCLUDE_DIRS})
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/clr")

target_link_libraries(${PROJECT_NAME} vw cs_common)
