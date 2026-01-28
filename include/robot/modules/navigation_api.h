#ifndef HUMANOID_ROBOT_INTERFACES_NAVIGATIONAPI
#define HUMANOID_ROBOT_INTERFACES_NAVIGATIONAPI

#include "robot/client/interfaces_client.h"
#include "ros2/action_msgs/GoalStatus.pb.h"
#include "ros2/geometry_msgs/Pose.pb.h"
#include "sdk_service/common/service.pb.h"
#include "sdk_service/navigation/req_pose_msg.pb.h"
#include "sdk_service/navigation/res_start_nav.pb.h"
#include "sdk_service/navigation/res_status.pb.h"

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

NavigationResStatus GetCurrentPose(std::unique_ptr<InterfacesClient> &client,
                                   const ReqPoseMsg &request_pose_msg,
                                   Pose &current_pose);
} // namespace navigation_api
} // namespace robot
} // namespace clientSDK
} // namespace humanoid_robot
#endif // HUMANOID_ROBOT_INTERFACES_NAVIGATIONAPI