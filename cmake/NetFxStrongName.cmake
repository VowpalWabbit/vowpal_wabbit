# A function to add StrongName Signing to the specified CMake TARGET
#
# target_name  : The CMake TARGET to update
# signing_key  : The Private Key file to use when StrongName signing 
function(strongname_sign_target target_name
                                signing_key)
  message (STATUS "Signing ${target_name} using key file: ${signing_key}")
  get_property(strongname_target_is_mixed_mode TARGET ${target_name} PROPERTY COMMON_LANGUAGE_RUNTIME SET)
  
  if (${strongname_target_is_mixed_mode})
    set_target_properties(${target_name}
      PROPERTIES
      VS_GLOBAL_LinkKeyFile ${signing_key})
  else()
    set_target_properties(${target_name}
      PROPERTIES
      VS_GLOBAL_SignAssembly "True"
      VS_GLOBAL_AssemblyOriginatorKeyFile ${signing_key})
  endif()
endfunction()