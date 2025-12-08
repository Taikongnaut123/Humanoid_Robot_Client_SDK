# Client-SDK Example CMakeLists.txt
cmake_minimum_required(VERSION 3.8)

project(interfaces_client_example LANGUAGES CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找依赖
# find_package(PkgConfig REQUIRED)
# 这两个结果不同
# find_package(Protobuf REQUIRED)
# find_package(protobuf REQUIRED)
# find_package(gRPC REQUIRED)
find_package(loc_msg REQUIRED)
find_package(rclcpp REQUIRED)
find_package(unique_identifier_msgs REQUIRED)
# 输出目录设置 - 使用父项目的 OUTPUT_BIN_DIR
if(NOT DEFINED OUTPUT_BIN_DIR)
    message(FATAL_ERROR "OUTPUT_BIN_DIR must be defined by parent project")
endif()

# 设置示例可执行文件的输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_BIN_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_BIN_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_BIN_DIR})

message(DEBUG "SDK-Client Examples output directory: ${OUTPUT_BIN_DIR}")

# 创建可执行文件
add_executable(interfaces_client_example
    interfaces_client_example.cpp
)


# 设置编译属性
set_target_properties(interfaces_client_example PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_BIN_DIR}
)

# 设置包含目录
target_include_directories(interfaces_client_example PRIVATE
    ${CLIENT_SDK_ROOT_INCLUDE_DIR}
    ${PB_INCLUDE_DIR}
    ${PB_UTILS_INCLUDE_DIR}
)

# 链接库
target_link_libraries(interfaces_client_example PRIVATE
    CHRIC_ClientSDK
    CHRIC_CommonLog
    CHRIC_PBUtils
    CHRIC_robotCommon
    libCHRIC_interfacesPB
    libCHRIC_commonPB
    protobuf::libprotobuf
    gRPC::grpc++
) 
    

# 创建导航客户端示例可执行文件
add_executable(navgation_client_example
    navgation_client_example.cpp
)

# 设置编译属性
set_target_properties(navgation_client_example PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_BIN_DIR}
)

# 设置包含目录
target_include_directories(navgation_client_example PRIVATE
    ${CLIENT_SDK_ROOT_INCLUDE_DIR}
    ${PB_INCLUDE_DIR}
    ${PB_UTILS_INCLUDE_DIR}
    ${loc_msg_INCLUDE_DIRS}
    ${rclcpp_INCLUDE_DIRS}
    ${unique_identifier_msgs_INCLUDE_DIRS}
    ${COMMON_INCLUDE_DIR}/communication
)

# 链接库
target_link_libraries(navgation_client_example PRIVATE
    CHRIC_ClientSDK
    CHRIC_CommonLog
    CHRIC_PBUtils
    CHRIC_robotCommon
    libCHRIC_interfacesPB
    libCHRIC_commonPB
    protobuf::libprotobuf
    gRPC::grpc++

    ${loc_msg_LIBRARIES}
    rclcpp::rclcpp
) 

install(TARGETS interfaces_client_example
        DESTINATION ${CHRIC_TERMINAL_FOLDER}/examples)

install(TARGETS navgation_client_example
        DESTINATION ${CHRIC_TERMINAL_FOLDER}/examples)
