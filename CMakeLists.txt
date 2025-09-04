cmake_minimum_required(VERSION 3.8)

project(HumanoidRobotSDK VERSION 1.0.0 LANGUAGES CXX)

# 使用CACHE方式定义选项，确保在子目录中也能正常工作
set(BUILD_SDK_CLIENT_EXAMPLES ON CACHE BOOL "Build SDK Client examples")
message(DEBUG "Building SDK-Client examples: ${BUILD_SDK_CLIENT_EXAMPLES}")

# 设置Client-SDK项目公共变量
set(CLIENT_SDK_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "Root path of the Client SDK project")
set(CLIENT_SDK_ROOT_INCLUDE_DIR "${CLIENT_SDK_ROOT_PATH}/include" CACHE PATH "Path to the Client SDK include files")
set(CLIENT_SDK_ROOT_LIB_DIR "${CLIENT_SDK_ROOT_PATH}/lib/${PLATFORM_ARCH}/${BUILD_TYPE_DIR}" CACHE PATH "Path to the Client SDK library files")

set(CLIENT_SDK_CLIENT_INCLUDE_DIR "${CLIENT_SDK_ROOT_INCLUDE_DIR}/robot/client" CACHE PATH "Path to the Client SDK client include files")
set(CLIENT_SDK_COMMON_INCLUDE_DIR "${CLIENT_SDK_ROOT_INCLUDE_DIR}" CACHE PATH "Path to the Client SDK common include files")
set(CLIENT_SDK_V1_INCLUDE_DIR "${CLIENT_SDK_ROOT_INCLUDE_DIR}/robot/v1" CACHE PATH "Path to the Client SDK V1 include files")

set(CLIENT_SDK_CLIENT_LIB_DIR "${CLIENT_SDK_ROOT_LIB_DIR}/robot/client" CACHE PATH "Path to the Client SDK client library files")
set(CLIENT_SDK_COMMON_LIB_DIR "${CLIENT_SDK_ROOT_LIB_DIR}/robot/common" CACHE PATH "Path to the Client SDK common library files")
set(CLIENT_SDK_V1_LIB_DIR "${CLIENT_SDK_ROOT_LIB_DIR}/robot/v1" CACHE PATH "Path to the Client SDK V1 library files")

# 设置SDK版本信息
set(HUMANOID_ROBOT_SDK_VERSION_MAJOR 1)
set(HUMANOID_ROBOT_SDK_VERSION_MINOR 0)
set(HUMANOID_ROBOT_SDK_VERSION_PATCH 0)
set(HUMANOID_ROBOT_SDK_VERSION "${HUMANOID_ROBOT_SDK_VERSION_MAJOR}.${HUMANOID_ROBOT_SDK_VERSION_MINOR}.${HUMANOID_ROBOT_SDK_VERSION_PATCH}")

# 添加子目录
add_subdirectory(source)

if(BUILD_SDK_CLIENT_EXAMPLES)
        message(DEBUG "Adding SDK-Client examples subdirectory...")
        add_subdirectory(example)
endif()

# 配置安装目标
include(GNUInstallDirs)

# 安装头文件
install(DIRECTORY include/
        DESTINATION ${CHRIC_TERMINAL_FOLDER}/include
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp")

# 安装文档
install(FILES 
        README.md
        DESIGN_DOCUMENT.md
        DESTINATION ${CHRIC_TERMINAL_FOLDER})

# 安装示例
install(FILES example/interfaces_client_example.cpp
        DESTINATION ${CHRIC_TERMINAL_FOLDER}/examples)