# Minimum requirements for cmake
cmake_minimum_required(VERSION 3.22)

# CMake helpers
add_subdirectory(cmake)

# Project name
project(lenny-communication)

# Build option
capitalize(LENNY_PROJECT ${PROJECT_NAME})
option(${LENNY_PROJECT}_BUILD_APPS "Build apps" ON)

option(LENNY_COMMUNICATION_USE_WEBRTC "Use WebRTC" OFF)
if(LENNY_COMMUNICATION_USE_WEBRTC)
	add_definitions(-DLENNY_COMMUNICATION_USE_WEBRTC)
endif()

# Project folder path
set(LENNY_PROJECT_FOLDER "\"${CMAKE_CURRENT_LIST_DIR}\"" CACHE STRING "")

# Generate log directory
file(MAKE_DIRECTORY "logs")

# CXX standard
set(CMAKE_CXX_STANDARD 20)

# Fetch lenny repository
fetch_lenny_repository(tools master)

# Source code
add_subdirectory(source)
