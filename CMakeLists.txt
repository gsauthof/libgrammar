cmake_minimum_required(VERSION 2.8)
project(libgrammar CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y -Wall")


include (${CMAKE_SOURCE_DIR}/cmake/modules/coverage.cmake)
include (${CMAKE_SOURCE_DIR}/cmake/modules/sanitize.cmake)


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/ragel")

find_package(RAGEL 6.6 REQUIRED)

find_package(Boost 1.55
  COMPONENTS
    filesystem
    system
  REQUIRED)

RAGEL_TARGET(asn1_mini_parser
  grammar/asn1/mini_parser.rl
  ${CMAKE_CURRENT_BINARY_DIR}/asn1_mini_parser.cc
  )

IF(${CMAKE_PROJECT_NAME} STREQUAL "libgrammar")
add_subdirectory(libixxx)
add_subdirectory(libixxxutil)
endif() # CMAKE_PROJECT_NAME

set(LIB_SRC 
  grammar/version.cc
  grammar/grammar.cc
  grammar/tsort.cc
  grammar/asn1/grammar.cc
  grammar/xml/printer.cc
  grammar/xml/xsd.cc
  grammar/xml/rng.cc
  grammar/graph/dot.cc
  grammar/tap/tap.cc
  ${RAGEL_asn1_mini_parser_OUTPUTS}
  )

add_library(grammar SHARED
  ${LIB_SRC}
  )
add_library(grammar_static STATIC
  ${LIB_SRC}
  )

# under windows shared/static libraries have the same extension ...
if(${UNIX})
  set_target_properties(grammar_static PROPERTIES OUTPUT_NAME grammar)
endif()

add_executable(ged
  ged/main.cc
  ged/arguments.cc
  )
set_property(TARGET ged grammar grammar_static PROPERTY INCLUDE_DIRECTORIES
  ${Boost_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/libixxx
  ${CMAKE_SOURCE_DIR}/libixxxutil
  )
target_link_libraries(ged
  grammar_static
  ixxxutil_static
  ixxx_static
  )

# guard from super-projects, i.e. when it is added as subdirectory
IF(${CMAKE_PROJECT_NAME} STREQUAL "libgrammar")

  add_executable(ut
    test/main.cc
    test/test.cc
    test/asn1/identity.cc
    test/asn1/compare.cc
    test/asn1/mini_parser.cc
    test/grammar.cc

    ${RAGEL_asn1_mini_parser_OUTPUTS}
    )
  set_property(TARGET ut PROPERTY INCLUDE_DIRECTORIES
    ${Boost_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/libixxx
    ${CMAKE_CURRENT_SOURCE_DIR}/libixxxutil
    )
  target_link_libraries(ut
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    grammar_static
    ixxxutil_static
    ixxx_static
    )

  add_custom_target(check COMMAND ut)

endif() # CMAKE_PROJECT_NAME

