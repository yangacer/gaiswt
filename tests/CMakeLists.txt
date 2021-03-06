
include_directories ( ${PROJECT_SOURCE_DIR} )

option (GAISWT_BUILD_UNIT_TESTS OFF "[Warning!] Time consuming build")

#add_executable ( connector connector.cpp )
#target_link_libraries ( connector gaiswt )

add_executable ( agent agent.cpp )
target_link_libraries ( agent gaiswt )

add_executable ( server server.cpp )
target_link_libraries ( server gaiswt )

add_executable ( mmstore mmstore.cpp )
target_link_libraries ( mmstore gaiswt )

add_executable ( asio_mmstore asio_mmstore.cpp )
target_link_libraries ( asio_mmstore gaiswt )

add_executable ( mmstore_overlap mmstore_overlap.cpp )
target_link_libraries ( mmstore_overlap gaiswt )
 
add_executable ( mmstore_serialize mmstore_serialize.cpp )
target_link_libraries ( mmstore_serialize gaiswt )

add_executable ( mmstore_import mmstore_import.cpp )
target_link_libraries ( mmstore_import gaiswt )

#add_executable ( mmstore_contention mmstore_contention.cpp )
#target_link_libraries ( mmstore_contention gaiswt )

add_executable ( connection connection.cpp )
target_link_libraries ( connection gaiswt )
 
add_executable ( uri_dispatcher uri_dispatcher.cpp )
target_link_libraries ( uri_dispatcher gaiswt )

if(GAISWT_BUILD_UNIT_TESTS)

  add_executable ( url_parser url_parser.cpp )
  add_executable ( uri_parser uri_parser.cpp )
  add_executable ( esc_char_parser esc_char_parser.cpp )

  add_executable ( esc_char_generator esc_char_generator.cpp )
  add_executable ( uri_generator uri_generator.cpp )

  add_executable ( parser parser.cpp )
  target_link_libraries ( parser gaiswt )

  add_executable ( request_generator request_generator.cpp)
  target_link_libraries (request_generator gaiswt)

  add_executable ( request_parser request_parser.cpp )
  target_link_libraries (request_parser gaiswt)

endif(GAISWT_BUILD_UNIT_TESTS)
