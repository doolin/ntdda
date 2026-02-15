# Unit tests for libdda
# Links against dda library and additional sources as needed

add_executable(constants_test constants_test.c unittest.c)
target_include_directories(constants_test PRIVATE
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/src/platform/stubs
)
target_link_libraries(constants_test PRIVATE dda)
target_compile_definitions(constants_test PRIVATE STANDALONE)
target_link_libraries(constants_test PRIVATE m)

# Tests that need extra sources (not fully in libdda API yet)
add_executable(geometrydata_test
  geometrydata_test.c
)
target_include_directories(geometrydata_test PRIVATE
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/src/platform/stubs
)
target_link_libraries(geometrydata_test PRIVATE dda)
target_compile_definitions(geometrydata_test PRIVATE STANDALONE)
target_link_libraries(geometrydata_test PRIVATE m)

add_executable(analysisdata_test
  analysisdata_test.c
)
target_include_directories(analysisdata_test PRIVATE
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/src/platform/stubs
)
target_link_libraries(analysisdata_test PRIVATE dda)
target_compile_definitions(analysisdata_test PRIVATE STANDALONE)
target_link_libraries(analysisdata_test PRIVATE m)

add_executable(bolttest bolttest.c)
target_include_directories(bolttest PRIVATE
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/src/platform/stubs
)
target_link_libraries(bolttest PRIVATE dda)
target_compile_definitions(bolttest PRIVATE STANDALONE)
target_link_libraries(bolttest PRIVATE m)

add_executable(ddadlist_test ddadlist_test.c)
target_include_directories(ddadlist_test PRIVATE
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/src/platform/stubs
)
target_link_libraries(ddadlist_test PRIVATE dda)
target_compile_definitions(ddadlist_test PRIVATE STANDALONE)

add_executable(inpoly_test inpoly_test.c)
target_include_directories(inpoly_test PRIVATE
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/src/platform/stubs
)
target_link_libraries(inpoly_test PRIVATE dda)
target_compile_definitions(inpoly_test PRIVATE STANDALONE)
target_link_libraries(inpoly_test PRIVATE m)

add_executable(material_test material_test.c)
target_include_directories(material_test PRIVATE
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/src/platform/stubs
)
target_link_libraries(material_test PRIVATE dda)
target_compile_definitions(material_test PRIVATE STANDALONE)
target_link_libraries(material_test PRIVATE m)

add_executable(stress_test stress_test.c)
target_include_directories(stress_test PRIVATE
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/src/platform/stubs
)
target_link_libraries(stress_test PRIVATE dda)
target_compile_definitions(stress_test PRIVATE STANDALONE)
target_link_libraries(stress_test PRIVATE m)

add_executable(loadpointtest loadpointtest.c)
target_include_directories(loadpointtest PRIVATE
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/src/platform/stubs
)
target_link_libraries(loadpointtest PRIVATE dda)
target_compile_definitions(loadpointtest PRIVATE STANDALONE)
target_link_libraries(loadpointtest PRIVATE m)

add_executable(matmulttest matmulttest.c)
target_include_directories(matmulttest PRIVATE
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/src/platform/stubs
)
target_link_libraries(matmulttest PRIVATE dda)
target_compile_definitions(matmulttest PRIVATE STANDALONE)
target_link_libraries(matmulttest PRIVATE m)

# all_tests - special build with ALL_TESTS (combines constants_test + bolttest)
add_executable(all_tests
  all_tests.c
  constants_test.c
  bolttest.c
  unittest.c
)
target_include_directories(all_tests PRIVATE
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/src/platform/stubs
)
target_link_libraries(all_tests PRIVATE dda)
target_compile_definitions(all_tests PRIVATE STANDALONE ALL_TESTS)
target_link_libraries(all_tests PRIVATE m)

# CTest integration
enable_testing()
add_test(NAME constants_test COMMAND constants_test)
add_test(NAME geometrydata_test COMMAND geometrydata_test)
add_test(NAME analysisdata_test COMMAND analysisdata_test)
add_test(NAME bolttest COMMAND bolttest)
add_test(NAME ddadlist_test COMMAND ddadlist_test)
add_test(NAME inpoly_test COMMAND inpoly_test)
add_test(NAME material_test COMMAND material_test)
add_test(NAME stress_test COMMAND stress_test)
add_test(NAME loadpointtest COMMAND loadpointtest)
add_test(NAME matmulttest COMMAND matmulttest)
add_test(NAME all_tests COMMAND all_tests)
