project(TCPTest_Server)

add_executable(${PROJECT_NAME} TCPServer.cpp)

target_link_libraries(${PROJECT_NAME}
        PUBLIC lenny::communication
        )

#-------------------------------------------------------------------

project(TCPTest_Client)

add_executable(${PROJECT_NAME} TCPClient.cpp)

target_link_libraries(${PROJECT_NAME}
        PUBLIC lenny::communication
        )