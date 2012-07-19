cmake_minimum_required (VERSION 2.8)
project (gaiswt)

set ( SOURCE 
  file_handler.cpp
  mime_types.cpp
  reply.cpp
  request_parser.cpp
  connector.cpp
  server.cpp
  agent.cpp )

set (Boost_ADDITIONAL_VERSIONS "1.50" "1.50.0" )
find_package ( Boost 1.50.0 COMPONENTS 
               chrono
               thread
               system )

if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
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
add_dependencies ( gaiswt ${Boost_LIBRARIES} )

# Tests
option (GAISWT_BUILD_TESTS "Build tests of gaiswt" OFF)

if(GAISWT_BUILD_TESTS)
  add_subdirectory ( tests )
endif()


