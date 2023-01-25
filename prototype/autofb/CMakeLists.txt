SET (VW_PROTO_AUTOFB_SOURCES
  autofb.cc
  main.cc
  kernel_svm.cc
  typeerase.cc
  typesys.cc

  manual_binding/kernel_svm_bindings.cc

  v2/auto_flatbuf.cc
  v2/auto_flatbuf_builder.cc
  v2/type_system.cc
  v2/test.cc
  v2/kernel_svm.cc
  v2/simple_reduction.cc
  v2/vwtypes_setup_base_i.cc
  v2/kernel_svm.h
  
  v3/test_v3.cc
  v3/autofb.cc
  v3/typesys.cc
  v3/vwtypes/abstractions.cc
  v3/reductions/simple/simple.cc
)

add_executable(vw_proto_autofb ${VW_PROTO_AUTOFB_SOURCES} "v3/type_erase.h" "v3/type_activation.h")

find_package(Flatbuffers CONFIG REQUIRED)

target_include_directories(vw_proto_autofb PUBLIC ${FLATBUFFERS_INCLUDE_DIR})

target_link_libraries(vw_proto_autofb
  flatbuffers::flatbuffers
)
