cmake_minimum_required(VERSION 3.8)
project(robot_client)
# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找依赖
find_package(PkgConfig REQUIRED)
find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)

# 集成 vcpkg
# 根目录已集成无需再设置
# if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)  
#     set(CMAKE_TOOLCHAIN_FILE "/home/ubuntu/vcpkg/scripts/buildsystems/vcpkg.cmake")
# endif()

add_library(robot_client SHARED
    interfaces_client.cpp)
message(CLIENT_SDK_COMMON_INCLUDE_DIR: ${CLIENT_SDK_COMMON_INCLUDE_DIR})

target_include_directories(
    robot_client
    PUBLIC
    ${CLIENT_SDK_ROOT_INCLUDE_DIR}  # 使用根包含目录，这样可以找到 robot/common/status.h
    ${PB_INCLUDE_DIR}  # PB 生成的头文件
)

# 链接必要的库
target_link_libraries(robot_client
    PUBLIC
    libinterfacesPB
    libcommonPB
    protobuf::libprotobuf
    gRPC::grpc++
)