cmake_minimum_required(VERSION 3.8)

add_subdirectory(robot)




set(DEPENDENCIES_INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/framework/include/common/idl/protobuf/interfaces
    ${CMAKE_SOURCE_DIR}/framework/include/common/idl/protobuf/ros2
    ${CMAKE_SOURCE_DIR}/framework/include/common/idl/protobuf/sdk_service
    ${CMAKE_SOURCE_DIR}/framework/include/common/idl/protobuf/common
)

install(DIRECTORY ${DEPENDENCIES_INCLUDE_DIRS}
    DESTINATION ${CHRIC_TERMINAL_FOLDER}/konka_sdk_client/include/protobuf
    )
install(FILES ${CMAKE_SOURCE_DIR}/framework/include/common/idl/protobuf/sdk_service/pb_export.h
    DESTINATION ${CHRIC_TERMINAL_FOLDER}/konka_sdk_client/include/protobuf
    )
# 安装framework库文件
file(GLOB DEPENDENCIES_LIBRARIES
    ${CMAKE_SOURCE_DIR}/framework/lib/libchric_protobuf_interfaces.so*
    ${CMAKE_SOURCE_DIR}/framework/lib/libchric_protobuf_ros2.so*
    ${CMAKE_SOURCE_DIR}/framework/lib/libchric_protobuf_sdk_service.so*
    ${CMAKE_SOURCE_DIR}/framework/lib/libchric_protobuf_common.so*
)
install(FILES ${DEPENDENCIES_LIBRARIES}
    DESTINATION ${CHRIC_TERMINAL_FOLDER}/konka_sdk_client/lib
    )

# 安装third_party文件
set(THIRD_PARTIES
    ${CMAKE_SOURCE_DIR}/framework/third_party/x86_64/release/grpc-1.71.0
)
install(DIRECTORY ${THIRD_PARTIES}
    DESTINATION ${CHRIC_TERMINAL_FOLDER}/konka_sdk_client/third_party
    )



