find_path (RAPIDJSON_INCLUDES
    NAMES rapidjson/rapidjson.h rapidjson/reader.h rapidjson/writer.h
    PATH_SUFFIXES include
    )

message (STATUS "Found components for RapidJSON")
message (STATUS "RAPIDJSON_INCLUDES  = ${RAPIDJSON_INCLUDES}")
