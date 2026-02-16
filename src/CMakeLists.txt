# libdda - DDA numerical core static library
# Builds without BLAS/LAPACK and without Win32 GUI code

find_package(LibXml2 REQUIRED)

set(LIBDDA_SRCS
  analysisdata.c
  analysisdriver.c
  analysisreader.c
  analysisddaml.c
  blockhandler.c
  blocks.c
  bolt.c
  combineddf.c
  contacts.c
  constants.c
  contact.c
  dda.c
  dda_error.c
  ddadlist.c
  ddafile.c
  ddaml.c
  ddamemory.c
  display.c
  dxf.c
  friction.c
  functions.c
  geomddaml.c
  geomdriver.c
  geometrydata.c
  geomreader.c
  ghssolver.c
  gravity.c
  inpoly.c
  joint.c
  loadmatrix.c
  loadpoint.c
  loghtml.c
  lns.c
  material.c
  mohrcoulomb.c
  postprocess.c
  rb.c
  replay.c
  sparsestorage.c
  stress.c
  timehistory.c
  timestep.c
  transfercontact.c
  utils.c
  graphics.c
)

add_library(dda STATIC ${LIBDDA_SRCS} platform/stubs/stubs.c)

target_include_directories(dda PRIVATE
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/platform/stubs
)

target_link_libraries(dda PUBLIC LibXml2::LibXml2)

# Do not define WIN32 - enables non-GUI code paths
target_compile_definitions(dda PRIVATE LIBXML_STATIC)

if(ENABLE_COVERAGE)
  target_compile_options(dda PRIVATE -fprofile-instr-generate -fcoverage-mapping)
  target_link_options(dda PRIVATE -fprofile-instr-generate)
endif()
