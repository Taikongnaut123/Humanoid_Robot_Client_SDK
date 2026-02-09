cmake_minimum_required(VERSION 3.8)
project("sdk_module_api" VERSION 1.0.0.0 DESCRIPTION "Client SDK - Robot Client Library" LANGUAGES CXX)
# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# 查找依赖
find_package(chric_protobuf_interfaces REQUIRED)
find_package(chric_protobuf_ros2 REQUIRED)
find_package(chric_protobuf_sdk_service REQUIRED)

message(STATUS "chric_protobuf_interfaces: ${chric_protobuf_interfaces_INCLUDE_DIRS}")
message(STATUS "chric_protobuf_ros2: ${chric_protobuf_ros2_INCLUDE_DIRS}")
message(STATUS "chric_protobuf_sdk_service: ${chric_protobuf_sdk_service_INCLUDE_DIRS}")

set(TARGET_NAME "chric_konka_sdk_module_api")

add_library(${TARGET_NAME} SHARED
    navigation_api.cpp
    control_api.cpp
    )

target_include_directories(
    ${TARGET_NAME}
    PUBLIC
    ${CLIENT_SDK_ROOT_INCLUDE_DIR}  # 使用根包含目录，这样可以找到 robot/common/status.h
)

ament_target_dependencies(${TARGET_NAME}
    "chric_protobuf_interfaces"
    "chric_protobuf_ros2"
    "chric_protobuf_sdk_service"
)

# 链接必要的库
target_link_libraries(${TARGET_NAME}
    protobuf::libprotobuf
    gRPC::grpc++
    gRPC::grpc++_reflection
)


set_target_properties(${TARGET_NAME}
    PROPERTIES
    OUTPUT_NAME "${TARGET_NAME}"
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_BIN_DIR}/konka_sdk_client/lib
    ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_BIN_DIR}/konka_sdk_client/lib
    RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_BIN_DIR}/konka_sdk_client/bin)


install(TARGETS ${TARGET_NAME}
    LIBRARY DESTINATION ${CHRIC_TERMINAL_FOLDER}/konka_sdk_client/lib
    ARCHIVE DESTINATION ${CHRIC_TERMINAL_FOLDER}/konka_sdk_client/lib
    RUNTIME DESTINATION ${CHRIC_TERMINAL_FOLDER}/konka_sdk_client/bin
    )