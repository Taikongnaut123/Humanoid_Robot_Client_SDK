
#include "robot/modules/navigation_api.h"

#include <iostream>
#include <string>

#include "common/service.pb.h"
#include "common/variant.pb.h"
#include "grpcpp/support/sync_stream.h"
#include "interfaces/interfaces_request_response.grpc.pb.h"
#include "interfaces/interfaces_request_response.pb.h"
#include "robot/client/interfaces_client.h"
#include "ros2/action_msgs/GoalStatus.pb.h"
#include "ros2/geometry_msgs/Pose.pb.h"
#include "sdk_service/common/service.pb.h"
#include "sdk_service/navigation/req_pose_msg.pb.h"
#include "sdk_service/navigation/res_start_nav.pb.h"
#include "sdk_service/navigation/res_status.pb.h"

#include "robot/common/json_convert_util.hpp"

namespace humanoid_robot {
namespace clientSDK {
namespace robot {
namespace navigation_api {
using ReqPoseMsg = humanoid_robot::PB::sdk_service::navigation::ReqPoseMsg;
using ResStartNav = humanoid_robot::PB::sdk_service::navigation::ResStartNav;
using NavigationResStatus =
    humanoid_robot::PB::sdk_service::navigation::ResStatus;
using Pose = humanoid_robot::PB::ros2::geometry_msgs::Pose;

using GoalStatus = humanoid_robot::PB::ros2::action_msgs::GoalStatus;
using InterfacesClient = humanoid_robot::clientSDK::robot::InterfacesClient;

using SendRequest = humanoid_robot::PB::interfaces::SendRequest;
using SendResponse = humanoid_robot::PB::interfaces::SendResponse;
using NavigationCommandCode =
    humanoid_robot::PB::sdk_service::common::NavigationCommandCode;

using Variant = humanoid_robot::PB::common::Variant;

NavigationResStatus GetCurrentPose(std::unique_ptr<InterfacesClient> &client,
                                   const ReqPoseMsg &request_pose_msg,
                                   Pose &current_pose) {
  NavigationResStatus res_status;
  res_status = NavigationResStatus::ERROR_DATA_GET_FAILED;

  SendRequest send_req;
  SendResponse send_resp;

  try {
    // Set command ID and input
    auto input_map = send_req.mutable_input()->mutable_keyvaluelist();
    {
      Variant command_id;
      command_id.set_int32value(NavigationCommandCode::kGetCurrentPose);
      input_map->insert(std::make_pair(std::string("command_id"), command_id));
    }
    {
      Variant request_dict;
      auto request_dict_map =
          request_dict.mutable_dictvalue()->mutable_keyvaluelist();
      Variant request_params;
      std::string serialize_data;

      auto serialize_status =
          request_pose_msg.SerializeToString(&serialize_data);
      if (!serialize_status) {
        std::cerr << "Failed to serialize ReqPoseMsg" << std::endl;
        return NavigationResStatus::ERROR_PARSE_FAILED;
      }
      request_params.set_bytevalue(serialize_data);
      request_dict_map->insert(
          std::make_pair(std::string("pose_request"), request_params));
      input_map->insert(std::make_pair(std::string("data"), request_dict));
    }

    std::unique_ptr<::grpc::ClientReaderWriter<SendRequest, SendResponse>>
        stream;
    std::unique_ptr<grpc::ClientContext> context;
    auto send_status = client->Send(stream, context, 10000);
    if (!send_status) {
      std::cerr << "Failed to create stream: " << send_status.message()
                << std::endl;
      return res_status;
    }

    if (!stream->Write(send_req)) {
      std::cerr << "Failed to write request" << std::endl;
      stream->WritesDone();
      stream->Finish();
      return res_status;
    }

    if (stream->Read(&send_resp)) {
      auto response_status = send_resp.ret();
      res_status =
          static_cast<NavigationResStatus>(std::stoi(response_status.code()));

      auto response_output = send_resp.output();
      auto data_it = response_output.keyvaluelist().find("data");
      if (data_it == response_output.keyvaluelist().end()) {
        std::cerr << "Failed to find data in response" << std::endl;
        return res_status;
      }
      const Variant &data_var = data_it->second;

      auto unserialize_status =
          current_pose.ParseFromString(data_var.bytevalue());
      if (!unserialize_status) {
        std::cerr << "Failed to serialize ReqPoseMsg" << std::endl;
        return NavigationResStatus::ERROR_PARSE_FAILED;
      }

    } else {
      std::cerr << "[✗] No Navigation response received" << std::endl;
      return res_status;
    }

    stream->WritesDone();
    auto finish_status = stream->Finish();
    return res_status;
    // context will be destroyed when unique_ptr goes out of scope
  } catch (const std::exception &e) {
    std::cerr << "Exception in navigation test: " << e.what() << std::endl;
    return res_status;
  }
}
} // namespace navigation_api
} // namespace robot
} // namespace clientSDK
} // namespace humanoid_robot