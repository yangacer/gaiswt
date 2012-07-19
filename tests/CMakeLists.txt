
include_directories ( ${PROJECT_SOURCE_DIR} )

add_executable ( connector connector.cpp )
target_link_libraries ( connector gaiswt ${Boost_LIBRARIES})

add_executable ( agent agent.cpp )
target_link_libraries ( agent gaiswt ${Boost_LIBRARIES})

add_executable ( server server.cpp )
target_link_libraries ( server gaiswt ${Boost_LIBRARIES} )

add_executable ( mmstore mmstore.cpp ../mmstore.cpp )
target_link_libraries ( mmstore ${Boost_LIBRARIES} )
