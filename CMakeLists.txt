cmake_minimum_required (VERSION 2.8)
project (gaiswt)

set ( SERVER_SOURCE
  server.cpp
  session.cpp
  uri_dispatcher.cpp)
      
set ( HANDLER_SOURCE
      handler.cpp
      mmstore_handler.cpp
      in_mem_handler.cpp
      server_handler.cpp)

set ( MMSTORE_SOURCE
      detail/region.cpp
      detail/map_ele.cpp
      detail/mmstore.cpp)

set ( SOURCE
  ${SERVER_SOURCE}
  ${HANDLER_SOURCE}
  ${MMSTORE_SOURCE}
  agent.cpp
  connection.cpp
  connection_manager.cpp
  entity.cpp
  parser.cpp
  generator.cpp
  utility.cpp)

set (Boost_ADDITIONAL_VERSIONS "1.51" "1.50.0" )
find_package ( Boost 1.51.0 REQUIRED COMPONENTS 
               chrono
               system
               serialization
               thread)

if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  link_directories(${Boost_LIBRARY_DIRS})
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
  message ("GNUCXX(g++) is detected. Apply C++0X flags.")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
  message ("GNUCXX(g++) is the only compler supported currently")
endif()

include ( rpath.cmake )

option (GAISWT_ENABLE_TRACKING "Tracking notification delivering" Off)
if( GAISWT_ENABLE_TRACKING )
  add_definitions ( -DOBSERVER_ENABLE_TRACKING )
endif()


# Library
option (GAISWT_BUILD_SHARED_LIB "Build shared gaiswt library" ON)
if( GAISWT_BUILD_SHARED_LIB )
  set(LIB_TYPE "SHARED")
else()
  set(LIB_TYPE "STATIC")
endif()

add_library ( gaiswt ${LIB_TYPE} ${SOURCE})
target_link_libraries ( gaiswt pthread ${Boost_LIBRARIES} )

# Tests
option (GAISWT_BUILD_TESTS "Build tests of gaiswt" OFF)

if(GAISWT_BUILD_TESTS)
  add_subdirectory ( tests )
endif()


