
add_executable ( connector connector.cpp )
target_link_libraries ( connector gaiswt ${Boost_LIBRARIES})

add_executable ( client client.cpp )
target_link_libraries ( client gaiswt ${Boost_LIBRARIES})

add_executable ( server server.cpp )
target_link_libraries ( server gaiswt ${Boost_LIBRARIES} )
