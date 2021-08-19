cmake_minimum_required(VERSION 3.5)
project(image-reaction VERSION 1.0)
set(PLUGIN_AUTHOR "scaled")
set(PLUGIN_GIT image-reaction)
set(LINUX_MAINTAINER_EMAIL "scaled@scaledteam.ru")
set(MACOS_BUNDLEID "ru.scaledteam.image-reaction")
set(CMAKE_CXX_STANDARD 11)

if (WIN32 OR APPLE)
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
endif()

if ("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
    add_definitions(-DLINUX=1)
    add_definitions(-DUNIX=1)
endif ()

find_package(LibObs REQUIRED)

set(image-reaction_SOURCES image-reaction.c)

add_library(image-reaction MODULE
        ${image-reaction_SOURCES})

target_link_libraries(image-reaction
    ${LIBOBS_LIBRARIES}
    ${OBS_FRONTEND_LIB})

include_directories("${LIBOBS_INCLUDE_DIR}/../UI/obs-frontend-api"
    ${LIBOBS_INCLUDE_DIR}
)
