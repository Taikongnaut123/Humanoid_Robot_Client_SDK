cmake_minimum_required(VERSION 3.8)
project(robot_v1)
# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 集成 vcpkg
# 根目录已集成无需再设置
# if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)  
#     set(CMAKE_TOOLCHAIN_FILE "/home/ubuntu/vcpkg/scripts/buildsystems/vcpkg.cmake")
# endif()

