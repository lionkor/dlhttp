cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

message(STATUS "dlhttp GitHub repository: https://github.com/lionkor/dlhttp")

project(dlhttp
    HOMEPAGE_URL https://github.com/lionkor/dlhttp
    VERSION 1.0.0
    LANGUAGES CXX)

find_package(Git)
option(GIT_SUBMODULE "Check submodules during build (requires git)" ON)
if (GIT_FOUND)
    # Update submodules as needed
    if(GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init --recursive failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
    endif()
endif()


# ------------------------ APPLE ---------------------------------
if(APPLE)
    set(DLHTTP_SOCKET_LIBS)
    # TODO
# ------------------------ WINDOWS ---------------------------------
elseif (WIN32)
    set(DLHTTP_SOCKET_LIBS Ws2_32)
    # TODO
# ------------------------ LINUX ---------------------------------
elseif (UNIX)
    set(DLHTTP_SOCKET_LIBS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")
    option(dlhttp_SANITIZE "Turns on thread and UB sanitizers" OFF)
    if (dlhttp_SANITIZE)
        message(STATUS "sanitize is ON")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined,thread")
    endif (dlhttp_SANITIZE)
endif ()

# ------------------------ C++ SETUP ---------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ------------------------ DEPENDENCIES ------------------------------
include(FindThreads)

message(STATUS "Adding local source dependencies")
add_subdirectory(deps)

# ------------------------ VARIABLES ---------------------------------


set(dlhttp_Sources
    include/dlhttp/dlhttp.h src/dlhttp.cpp
)

set(dlhttp_PublicIncludes
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/deps/ThreadPool"
)

set(dlhttp_PrivateIncludes
    "${CMAKE_CURRENT_SOURCE_DIR}/include/dlhttp"
)

set(dlhttp_Definitions)

set(dlhttp_Libraries
    doctest::doctest
    Threads::Threads
    ${DLHTTP_SOCKET_LIBS}
)

# ------------------------ dlhttp -----------------------------

add_library(dlhttp
    ${dlhttp_Sources}
)

target_compile_definitions(dlhttp PRIVATE
    ${dlhttp_Definitions}
    DOCTEST_CONFIG_DISABLE
)

target_include_directories(dlhttp PUBLIC 
    ${dlhttp_PublicIncludes}
)

target_include_directories(dlhttp PRIVATE 
    ${dlhttp_PrivateIncludes}
)

target_link_libraries(dlhttp
    ${dlhttp_Libraries}
    ${dlhttp_PlatformLibs}
)

# ------------------------ dlhttp TESTS -----------------------

option(BUILD_TESTS "Build dlhttp tests" ON)

if(BUILD_TESTS)
    add_executable(dlhttp-tests
        test/test_main.cpp
        ${dlhttp_Sources}
    )

    target_compile_definitions(dlhttp-tests PRIVATE
        ${dlhttp_Definitions}
    )

    target_include_directories(dlhttp-tests PUBLIC
        ${dlhttp_PublicIncludes}
    )

    target_include_directories(dlhttp-tests PRIVATE 
        ${dlhttp_PrivateIncludes}
    )

    target_link_libraries(dlhttp-tests
        ${dlhttp_Libraries}
        ${dlhttp_PlatformLibs}
    )
endif()

