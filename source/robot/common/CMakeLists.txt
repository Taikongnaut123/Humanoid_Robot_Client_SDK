cmake_minimum_required(VERSION 3.8)

set(TARGET_NAME CHRIC_robotCommon)

project(${TARGET_NAME} VERSION 1.0.0.0 DESCRIPTION "Client SDK - Robot Common Library" LANGUAGES CXX)
# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 集成 vcpkg
# 根目录已集成无需再设置
# if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)  
#     set(CMAKE_TOOLCHAIN_FILE "/home/ubuntu/vcpkg/scripts/buildsystems/vcpkg.cmake")
# endif()


add_library(${TARGET_NAME} SHARED
    status.cpp
    success_condition.cpp)

set_target_properties(${TARGET_NAME}
    PROPERTIES
    OUTPUT_NAME "${TARGET_NAME}"
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER "status.h;success_condition.h"
    RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_BIN_DIR})

target_include_directories(
    ${TARGET_NAME}
    PUBLIC
    ${CLIENT_SDK_COMMON_INCLUDE_DIR}
)

install(TARGETS ${TARGET_NAME}
    LIBRARY DESTINATION ${CHRIC_TERMINAL_FOLDER}/lib
    ARCHIVE DESTINATION ${CHRIC_TERMINAL_FOLDER}/lib
    RUNTIME DESTINATION ${CHRIC_TERMINAL_FOLDER}/bin
    # PUBLIC_HEADER DESTINATION ${CHRIC_TERMINAL_FOLDER}/include/robot/common
    )
