cmake_minimum_required(VERSION 3.8)

option(BUILD_CLIENT "build robot's client components" ON)
option(BUILD_COMMON  "build robot's common components" ON)
option(BUILD_V1 "build robot's V1 components" ON)

if(BUILD_CLIENT)
    message(STATUS "Adding Client SDK subdirectory...")
    add_subdirectory(client)
endif()

if(BUILD_COMMON)
    message(STATUS "Adding Common SDK subdirectory...")
    add_subdirectory(common)
endif()

if(BUILD_V1)
    message(STATUS "Adding SDK V1 subdirectory...")
    add_subdirectory(v1)
endif()