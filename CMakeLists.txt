cmake_minimum_required (VERSION 2.8)
project (gaiswt)

set ( SOURCE 
  file_handler.cpp
  mime_types.cpp
  reply.cpp
  request_parser.cpp
  connector.cpp
  server.cpp
  client.cpp )

find_package ( Boost 1.50.0 COMPONENTS 
               chrono
               thread
               system )

if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

add_library ( gaiswt ${SOURCE} )
add_dependencies ( gaiswt ${Boost_LIBRARIES} )

if(GAISWT_BUILD_TESTS)
  add_subdirectory ( tests )
endif()


