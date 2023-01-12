# External
add_subdirectory(external)

# Project
project(communication)

file(GLOB sources
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
        )

add_library(${PROJECT_NAME} ${sources})
add_library(lenny::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
        PUBLIC lenny::tools
        )

if (WIN32)
    target_link_libraries(${PROJECT_NAME}
            PUBLIC ws2_32
            )
endif ()

if (LENNY_COMMUNICATION_USE_WEBRTC)
    target_link_libraries(${PROJECT_NAME}
            PUBLIC LibDataChannel::LibDataChannel
            )
endif ()

target_include_directories(${PROJECT_NAME}
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
        )

target_compile_definitions(${PROJECT_NAME}
        PUBLIC LENNY_PROJECT_FOLDER=${LENNY_PROJECT_FOLDER}
        )

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)