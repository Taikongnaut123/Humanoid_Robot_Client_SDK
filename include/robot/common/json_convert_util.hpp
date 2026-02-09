#ifndef HUMANOID_ROBOT_JSON_CONVERT_UTIL_HPP
#define HUMANOID_ROBOT_JSON_CONVERT_UTIL_HPP

#include "google/protobuf/util/json_util.h"
namespace humanoid_robot {
namespace konka_sdk {
namespace common {
inline const google::protobuf::util::JsonPrintOptions &GetJsonPrintOptions() {
  static const google::protobuf::util::JsonPrintOptions json_print_options =
      []() {
        google::protobuf::util::JsonPrintOptions options;
        options.add_whitespace = true; // 格式化输出（和ROS2一致）
        options.always_print_fields_with_no_presence =
            false; // 省略默认值（和ROS2一致）
        options.always_print_enums_as_ints = true; // 枚举输出数字（核心对齐项）
        options.preserve_proto_field_names =
            true; // 保留下划线字段名（核心对齐项）
        options.unquote_int64_if_possible = true; // int64输出数字（和ROS2一致）
        return options;
      }();
  return json_print_options;
}

inline const google::protobuf::util::JsonParseOptions &GetJsonParseOptions() {
  static const google::protobuf::util::JsonParseOptions json_parse_options =
      []() {
        google::protobuf::util::JsonParseOptions options;
        options.ignore_unknown_fields = true;
        return options;
      }();
  return json_parse_options;
}
} // namespace common
} // namespace konka_sdk
} // namespace humanoid_robot

#endif