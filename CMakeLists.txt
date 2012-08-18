cmake_minimum_required (VERSION 2.8)
project (gaiswt)

set ( SOURCE
  request_handler.cpp
  mime_types.cpp
  reply.cpp
  request_parser.cpp
  connection.cpp
  server.cpp
  agent.cpp
  mmstore_handler.cpp
  in_mem_handler.cpp
  entity.cpp
  parser.cpp
  generator.cpp
  mmstore.cpp
  region.cpp
  utility.cpp)

set (Boost_ADDITIONAL_VERSIONS "1.51" "1.50.0" )
find_package ( Boost 1.51.0 REQUIRED COMPONENTS 
               chrono
               system 
               thread)

if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  link_directories(${Boost_LIBRARY_DIRS})
endif()

include ( rpath.cmake )

# Library
option (GAISWT_BUILD_SHARED_LIB "Build shared gaiswt library" ON)
if( GAISWT_BUILD_SHARED_LIB )
  set(LIB_TYPE "SHARED")
else()
  set(LIB_TYPE "STATIC")
endif()

add_library ( gaiswt ${LIB_TYPE} ${SOURCE} )
target_link_libraries ( gaiswt pthread ${Boost_LIBRARIES} )

# Tests
option (GAISWT_BUILD_TESTS "Build tests of gaiswt" OFF)

if(GAISWT_BUILD_TESTS)
  add_subdirectory ( tests )
endif()


