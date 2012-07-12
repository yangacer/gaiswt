cmake_minimum_required (VERSION 2.8)
project (gaiswt)

set ( SOURCE 
  file_handler.cpp
  main.cpp
  mime_types.cpp
  reply.cpp
  request_parser.cpp
  server.cpp
  )

include_directories ( /opt/include )
link_directories ( /opt/lib )

add_executable ( server ${SOURCE} )
target_link_libraries ( server boost_system boost_thread boost_chrono)

add_executable ( client client.cpp )
target_link_libraries ( client boost_system boost_thread boost_chrono)

