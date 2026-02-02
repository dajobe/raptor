# tests/fuzz/CMakeLists.txt

IF(NOT RAPTOR_ENABLE_FUZZING)
  RETURN()
ENDIF()

IF(NOT RAPTOR_FUZZER_FLAGS)
  MESSAGE(STATUS "RAPTOR_ENABLE_FUZZING is ON but RAPTOR_FUZZER_FLAGS is empty; skipping fuzz targets")
  RETURN()
ENDIF()

# add_fuzz_target(name parser_name)
# Creates a fuzz target binary from the generic fuzz_parser.c harness
# compiled with FUZZ_PARSER_NAME set to the given parser name.
FUNCTION(ADD_FUZZ_TARGET name parser_name)
  ADD_EXECUTABLE(${name} fuzz_parser.c)
  TARGET_INCLUDE_DIRECTORIES(${name} PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_BINARY_DIR}/src)
  TARGET_LINK_LIBRARIES(${name} raptor2)
  TARGET_COMPILE_DEFINITIONS(${name} PRIVATE
    RAPTOR_USE_LIBFUZZER=1
    FUZZ_PARSER_NAME="${parser_name}"
  )
  SET_TARGET_PROPERTIES(${name} PROPERTIES
    COMPILE_FLAGS "${RAPTOR_FUZZER_FLAGS}"
    LINK_FLAGS "${RAPTOR_FUZZER_FLAGS}"
    BUILD_RPATH "${CMAKE_BINARY_DIR}/src"
  )
ENDFUNCTION()

ADD_FUZZ_TARGET(fuzz_turtle turtle)
ADD_FUZZ_TARGET(fuzz_ntriples ntriples)
ADD_FUZZ_TARGET(fuzz_rdfxml rdfxml)
