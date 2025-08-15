# Client-SDK Example CMakeLists.txt
cmake_minimum_required(VERSION 3.8)

project(ClientSDKExamples LANGUAGES CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找依赖
find_package(PkgConfig REQUIRED)
find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)

# 输出目录设置 - 使用父项目的 OUTPUT_BIN_DIR
if(NOT DEFINED OUTPUT_BIN_DIR)
    message(FATAL_ERROR "OUTPUT_BIN_DIR must be defined by parent project")
endif()

# 设置示例可执行文件的输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_BIN_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_BIN_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_BIN_DIR})

message(STATUS "Client-SDK Examples output directory: ${OUTPUT_BIN_DIR}")

# 包含目录
include_directories(
    ${CLIENT_SDK_ROOT_INCLUDE_DIR}
    ${PB_INCLUDE_DIR}
    ${Protobuf_INCLUDE_DIRS}
    ${PB_UTILS_INCLUDE_DIR}
)

# 查找所有示例源文件
file(GLOB EXAMPLE_SOURCES "*.cpp")

# 为每个示例源文件创建可执行文件
foreach(EXAMPLE_SOURCE ${EXAMPLE_SOURCES})
    # 获取文件名（不含扩展名）
    get_filename_component(EXAMPLE_NAME ${EXAMPLE_SOURCE} NAME_WE)
    
    # 创建可执行文件
    add_executable(${EXAMPLE_NAME} ${EXAMPLE_SOURCE})
    
    # 设置编译属性
    set_target_properties(${EXAMPLE_NAME} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_BIN_DIR}
    )
    
    # 链接库
    target_link_libraries(${EXAMPLE_NAME}
    PUBLIC
        robot_client
        robot_common
        libinterfacesPB
        libcommonPB
        libPBUtils
        protobuf::libprotobuf
        gRPC::grpc++
    PRIVATE
        CHRIC_Common_Log
    )
    
    # 添加依赖关系
    add_dependencies(${EXAMPLE_NAME} robot_client robot_common libinterfacesPB libcommonPB libPBUtils) 
    
    message(STATUS "Created example: ${EXAMPLE_NAME}")
endforeach()

# 显示示例信息
message(STATUS "Client-SDK Examples configuration completed")
message(STATUS "Found example sources: ${EXAMPLE_SOURCES}")
message(STATUS "Examples will be built to: ${OUTPUT_BIN_DIR}")
