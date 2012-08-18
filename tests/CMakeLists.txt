
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

add_executable ( region_impl region_impl.cpp)
target_link_libraries ( region_impl gaiswt )
 
add_executable ( parser parser.cpp )
target_link_libraries ( parser gaiswt )

add_executable ( request_generator request_generator.cpp)
target_link_libraries (request_generator gaiswt)

add_executable ( request_parser request_parser.cpp )
target_link_libraries (request_parser gaiswt)

if(GAISWT_BUILD_UNIT_TESTS)

  add_executable ( url_parser url_parser.cpp )
  add_executable ( uri_parser uri_parser.cpp )
  add_executable ( esc_char_parser esc_char_parser.cpp )

  add_executable ( esc_char_generator esc_char_generator.cpp )
  add_executable ( uri_generator uri_generator.cpp )

endif(GAISWT_BUILD_UNIT_TESTS)
