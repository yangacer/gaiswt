
include_directories ( ${PROJECT_SOURCE_DIR} )

add_executable ( connector connector.cpp )
target_link_libraries ( connector gaiswt )

add_executable ( agent agent.cpp )
target_link_libraries ( agent gaiswt )

add_executable ( agent2 agent2.cpp )
target_link_libraries ( agent2 gaiswt )

add_executable ( server server.cpp )
target_link_libraries ( server gaiswt )

add_executable ( mmstore mmstore.cpp )
target_link_libraries ( mmstore gaiswt )

add_executable ( region_impl region_impl.cpp)
target_link_libraries ( region_impl gaiswt )
 
add_executable ( parser parser.cpp )
target_link_libraries ( parser gaiswt )


