SET (VW_PROTO_AUTOFB_SOURCES
  autofb.cc
  main.cc
  typeerase.cc
  typesys.cc
)

add_executable(vw_proto_autofb ${VW_PROTO_AUTOFB_SOURCES})

#find_package(flatbuffers REQUIRED)

#target_include_directories(vw_proto_autofb PUBLIC ${FLATBUFFERS_INCLUDE_DIR})

#target_link_libraries(vw_proto_autofb
#    #flatbuffers
#  )
