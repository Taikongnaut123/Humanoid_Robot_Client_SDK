cmake_minimum_required(VERSION 3.8)

set(TARGET_NAME CHRIC_ClientSDK)

project(${TARGET_NAME} VERSION 1.0.0.0 DESCRIPTION "Client SDK - Robot Client Library" LANGUAGES CXX)
# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找依赖
find_package(PkgConfig REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

# 集成 vcpkg
# 根目录已集成无需再设置
# if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)  
#     set(CMAKE_TOOLCHAIN_FILE "/home/ubuntu/vcpkg/scripts/buildsystems/vcpkg.cmake")
# endif()

add_library(${TARGET_NAME} SHARED
    interfaces_client.cpp
    client_callback_server.cpp)
    
message(CLIENT_SDK_COMMON_INCLUDE_DIR: ${CLIENT_SDK_COMMON_INCLUDE_DIR})

set_target_properties(${TARGET_NAME}
    PROPERTIES
    OUTPUT_NAME "${TARGET_NAME}"
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER "interfaces_client.h;client_callback_server.h"
    RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_BIN_DIR})

target_include_directories(
    ${TARGET_NAME}
    PUBLIC
    ${CLIENT_SDK_ROOT_INCLUDE_DIR}  # 使用根包含目录，这样可以找到 robot/common/status.h
    ${PB_INCLUDE_DIR}  # PB 生成的头文件
)

# 链接必要的库
target_link_libraries(${TARGET_NAME}
    PUBLIC
    libinterfacesPB
    libcommonPB
    protobuf::libprotobuf
    gRPC::grpc++
)