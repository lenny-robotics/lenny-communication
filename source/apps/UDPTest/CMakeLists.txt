project(UDPTest_Server)

add_executable(${PROJECT_NAME} UDPServer.cpp)

target_link_libraries(${PROJECT_NAME}
        PUBLIC lenny::communication
        )

#-------------------------------------------------------------------

project(UDPTest_Client)

add_executable(${PROJECT_NAME} UDPClient.cpp)

target_link_libraries(${PROJECT_NAME}
        PUBLIC lenny::communication
        )