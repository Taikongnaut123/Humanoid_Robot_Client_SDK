#include "robot/modules/navigation_api.h"

#include <iostream>
#include <stdexcept>
#include <string>

#include "common/service.pb.h"
#include "common/variant.pb.h"
#include "grpcpp/support/sync_stream.h"
#include "interfaces/interfaces_request_response.grpc.pb.h"
#include "interfaces/interfaces_request_response.pb.h"
#include "robot/client/interfaces_client.h"
#include "robot/common/json_convert_util.hpp"
#include "ros2/action_msgs/GoalStatus.pb.h"
#include "ros2/geometry_msgs/Pose.pb.h"
#include "ros2/nav_msgs/Goals.pb.h"
#include "ros2/nav_msgs/OccupancyGrid.pb.h"
#include "sdk_service/common/service.pb.h"
#include "sdk_service/navigation/req_pose_msg.pb.h"
#include "sdk_service/navigation/request_cancel_navigation.pb.h"
#include "sdk_service/navigation/request_charging.pb.h"
#include "sdk_service/navigation/request_grid_map.pb.h"
#include "sdk_service/navigation/request_remaining_distance.pb.h"
#include "sdk_service/navigation/res_start_nav.pb.h"
#include "sdk_service/navigation/res_status.pb.h"

namespace humanoid_robot {
namespace clientSDK {
namespace robot {
namespace navigation_api {

/**
 * @brief 导航API公共常量定义
 */
namespace constants {
// gRPC请求默认超时时间(ms)
constexpr int kDefaultGrpcTimeoutMs = 30000;
// 请求参数key
constexpr const char* kCommandIdKey = "command_id";
constexpr const char* kDataKey = "data";
constexpr const char* kRequestDataKey = "request_data";
// 错误提示信息
constexpr const char* kSerializeFailedMsg = "Failed to serialize request_data";
constexpr const char* kUnserializeFailedMsg =
    "Failed to unserialize response_data";
constexpr const char* kCreateStreamFailedMsg = "Failed to create stream: ";
constexpr const char* kSendRequestFailedMsg = "Failed to send request";
constexpr const char* kNoResponseReceivedMsg =
    "[✗] No Navigation response received";
constexpr const char* kDataKeyNotFoundMsg = "Failed to find data in response";
constexpr const char* kExceptionMsg = "Exception in navigation API: ";
}  // namespace constants

// 核心类型别名（集中管理，便于修改）
using NavigationCommandCode =
    humanoid_robot::PB::sdk_service::common::NavigationCommandCode;
using NavigationResStatus =
    humanoid_robot::PB::sdk_service::navigation::ResStatus;
using Variant = humanoid_robot::PB::common::Variant;
using SendRequest = humanoid_robot::PB::interfaces::SendRequest;
using SendResponse = humanoid_robot::PB::interfaces::SendResponse;
using InterfacesClient = humanoid_robot::clientSDK::robot::InterfacesClient;

// 业务数据类型别名
using ReqPoseMsg = humanoid_robot::PB::sdk_service::navigation::ReqPoseMsg;
using RequestGridMap =
    humanoid_robot::PB::sdk_service::navigation::RequestGridMap;
using Pose = humanoid_robot::PB::ros2::geometry_msgs::Pose;
using OccupancyGrid = humanoid_robot::PB::ros2::nav_msgs::OccupancyGrid;
using ResRemainingDistance =
    humanoid_robot::PB::sdk_service::navigation::ResponseRemainingDistance;
using RequestRemainingDistance =
    humanoid_robot::PB::sdk_service::navigation::RequestRemainingDistance;
using RequestCancelNavigation =
    humanoid_robot::PB::sdk_service::navigation::RequestCancelNavigation;
using ResponseCancelNavigation =
    humanoid_robot::PB::sdk_service::navigation::ResponseCancelNavigation;
using RequestStartCharging =
    humanoid_robot::PB::sdk_service::navigation::RequestStartCharging;
using ResponseStartCharging =
    humanoid_robot::PB::sdk_service::navigation::ResponseStartCharging;
using RequestStopCharging =
    humanoid_robot::PB::sdk_service::navigation::RequestStopCharging;
using ResponseStopCharging =
    humanoid_robot::PB::sdk_service::navigation::ResponseStopCharging;

// gRPC流类型别名
using GrpcStreamPtr =
    std::unique_ptr<::grpc::ClientReaderWriter<SendRequest, SendResponse>>;
using GrpcContextPtr = std::unique_ptr<grpc::ClientContext>;

// ===================== 封装公共工具函数 =====================
/**
 * @brief 检查protobuf序列化状态，失败则打印日志并返回false
 * @param serialize_status 序列化操作的返回值
 * @param error_msg 失败时的提示信息
 * @return 序列化是否成功
 */
inline bool CheckSerializeStatus(bool serialize_status,
                                 const std::string& error_msg) {
  if (!serialize_status) {
    std::cerr << error_msg << std::endl;
  }
  return serialize_status;
}

/**
 * @brief 构建gRPC请求对象（填充command_id和request_data）
 * @param command_id 导航命令ID
 * @param request_data 业务请求数据（protobuf对象）
 * @return 构建好的SendRequest对象
 */
template <typename RequestType>
SendRequest BuildSendRequest(NavigationCommandCode command_id,
                             const RequestType& request_data) {
  SendRequest send_req;
  auto input_map = send_req.mutable_input()->mutable_keyvaluelist();

  // 设置command_id
  Variant command_id_var;
  command_id_var.set_int32value(command_id);
  input_map->emplace(constants::kCommandIdKey, std::move(command_id_var));

  // 设置request_data
  Variant request_dict;
  auto request_dict_map =
      request_dict.mutable_dictvalue()->mutable_keyvaluelist();
  Variant request_params;

  std::string serialize_data;
  if (!CheckSerializeStatus(request_data.SerializeToString(&serialize_data),
                            constants::kSerializeFailedMsg)) {
    return send_req;  // 序列化失败，返回空请求
  }

  request_params.set_bytevalue(serialize_data);
  request_dict_map->emplace(constants::kRequestDataKey,
                            std::move(request_params));
  input_map->emplace(constants::kDataKey, std::move(request_dict));

  return send_req;
}

/**
 * @brief 发送gRPC请求并获取响应（核心通信逻辑封装）
 * @param client InterfacesClient对象
 * @param send_req 待发送的请求
 * @param send_resp 输出参数，接收响应
 * @return 是否成功获取响应
 */
bool SendGrpcRequest(std::unique_ptr<InterfacesClient>& client,
                     const SendRequest& send_req, SendResponse& send_resp) {
  GrpcStreamPtr stream;
  GrpcContextPtr context;

  // 创建gRPC流
  auto send_status =
      client->Send(stream, context, constants::kDefaultGrpcTimeoutMs);
  if (!send_status) {
    std::cerr << constants::kCreateStreamFailedMsg << send_status.message()
              << std::endl;
    return false;
  }

  // 发送请求
  if (!stream->Write(send_req)) {
    std::cerr << constants::kSendRequestFailedMsg << std::endl;
    stream->WritesDone();
    stream->Finish();
    return false;
  }

  // 读取响应
  if (!stream->Read(&send_resp)) {
    std::cerr << constants::kNoResponseReceivedMsg << std::endl;
    stream->WritesDone();
    stream->Finish();
    return false;
  }

  // 完成流操作并释放资源
  stream->WritesDone();
  stream->Finish();
  return true;
}

/**
 * @brief 解析gRPC响应数据到目标对象
 * @param send_resp gRPC响应对象
 * @param result 输出参数，接收解析后的数据
 * @return 导航响应状态
 */
template <typename ResultType>
NavigationResStatus ParseResponse(const SendResponse& send_resp,
                                  ResultType& result) {
  // 获取响应状态码
  auto response_status = send_resp.ret();
  NavigationResStatus res_status =
      static_cast<NavigationResStatus>(std::stoi(response_status.code()));

  // 非成功状态直接返回
  if (res_status != NavigationResStatus::RESPONSE_SUCCESS) {
    std::cerr << "Request failed: " << response_status.message() << std::endl;
    return res_status;
  }

  // 查找并解析data字段
  auto response_output = send_resp.output();
  auto data_it = response_output.keyvaluelist().find(constants::kDataKey);
  if (data_it == response_output.keyvaluelist().end()) {
    std::cerr << constants::kDataKeyNotFoundMsg << std::endl;
    return NavigationResStatus::ERROR_DATA_GET_FAILED;
  }

  // 反序列化数据
  const Variant& data_var = data_it->second;
  if (!CheckSerializeStatus(result.ParseFromString(data_var.bytevalue()),
                            constants::kUnserializeFailedMsg)) {
    return NavigationResStatus::ERROR_PARSE_FAILED;
  }

  return NavigationResStatus::RESPONSE_SUCCESS;
}

/**
 * @brief 通用导航请求模板函数（核心逻辑复用）
 * @param client InterfacesClient对象
 * @param command_id 导航命令ID
 * @param request_data 业务请求数据
 * @param result 输出参数，接收响应数据
 * @return 导航响应状态
 */
template <typename RequestType, typename ResultType>
NavigationResStatus NavigationRequestTemplate(
    std::unique_ptr<InterfacesClient>& client, NavigationCommandCode command_id,
    const RequestType& request_data, ResultType& result) {
  NavigationResStatus res_status = NavigationResStatus::ERROR_DATA_GET_FAILED;

  try {
    // 1. 构建请求
    SendRequest send_req = BuildSendRequest(command_id, request_data);
    // 序列化失败时直接返回
    if (send_req.input().keyvaluelist().empty()) {
      return NavigationResStatus::ERROR_PARSE_FAILED;
    }

    // 2. 发送gRPC请求
    SendResponse send_resp;
    if (!SendGrpcRequest(client, send_req, send_resp)) {
      return res_status;
    }

    // 3. 解析响应
    res_status = ParseResponse(send_resp, result);

  } catch (const std::exception& e) {
    std::cerr << constants::kExceptionMsg << e.what() << std::endl;
  }

  return res_status;
}

NavigationResStatus GetCurrentPose(std::unique_ptr<InterfacesClient>& client,
                                   const ReqPoseMsg& request_data,
                                   Pose& current_pose) {
  return NavigationRequestTemplate(client,
                                   NavigationCommandCode::kGetCurrentPose,
                                   request_data, current_pose);
}

NavigationResStatus GetGridMap2D(std::unique_ptr<InterfacesClient>& client,
                                 const RequestGridMap& request_data,
                                 OccupancyGrid& occupancy_grid_map) {
  return NavigationRequestTemplate(client, NavigationCommandCode::kGetGridMap2D,
                                   request_data, occupancy_grid_map);
}
NavigationResStatus NavigationTo(std::unique_ptr<InterfacesClient>& client,
                                 const Goals& goals,
                                 ResStartNav& res_start_nav) {
  return NavigationRequestTemplate(client, NavigationCommandCode::kNavigationTo,
                                   goals, res_start_nav);
}
NavigationResStatus GetRemainingPathDistance(
    std::unique_ptr<InterfacesClient>& client,
    const RequestRemainingDistance& request,
    ResponseRemainingDistance& remaining_distance) {
  return NavigationRequestTemplate(
      client, NavigationCommandCode::kGetRemainingPathDistance, request,
      remaining_distance);
}

NavigationResStatus CancelNavigationTask(
    std::unique_ptr<InterfacesClient>& client,
    const RequestCancelNavigation& request,
    ResponseCancelNavigation& cancel_status) {
  return NavigationRequestTemplate(client,
                                   NavigationCommandCode::kCancelNavigationTask,
                                   request, cancel_status);
}
NavigationResStatus StartChargingTask(std::unique_ptr<InterfacesClient>& client,
                                      const RequestStartCharging& request,
                                      ResponseStartCharging& start_charging) {
  return NavigationRequestTemplate(
      client, NavigationCommandCode::kStartCharging, request, start_charging);
}
NavigationResStatus StopChargingTask(std::unique_ptr<InterfacesClient>& client,
                                     const RequestStopCharging& request,
                                     ResponseStopCharging& stop_charging) {
  return NavigationRequestTemplate(client, NavigationCommandCode::kStopCharging,
                                   request, stop_charging);
}

}  // namespace navigation_api
}  // namespace robot
}  // namespace clientSDK
}  // namespace humanoid_robot