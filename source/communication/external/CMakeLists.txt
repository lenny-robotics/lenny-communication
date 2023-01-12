# libdatachannel
if (LENNY_COMMUNICATION_USE_WEBRTC AND NOT TARGET libdatachannel)
    FetchContent_Declare(
            libdatachannel #
            GIT_REPOSITORY https://github.com/paullouisageneau/libdatachannel.git
            GIT_TAG v0.18.0 #
    )
    fetch_repository(libdatachannel)
    option(NO_EXAMPLES "Disable examples" ON)
    option(NO_TESTS "Disable tests build" ON)
    set(CMAKE_PROJECT_VERSION 2.4.2) #To avoid problem with libsrtp
    add_subdirectory(${libdatachannel_SOURCE_DIR} libdatachannel)
    unset(CMAKE_PROJECT_VERSION)
endif ()