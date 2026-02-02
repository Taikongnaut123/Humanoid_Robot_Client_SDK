#ifndef HUMANOID_ROBOT_INTERFACES_NAVIGATIONAPI
#define HUMANOID_ROBOT_INTERFACES_NAVIGATIONAPI

#include "interfaces/interfaces_request_response.pb.h"
#include "robot/client/interfaces_client.h"
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
#include "sdk_service/navigation/request_charging.pb.h"

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

using RequestGridMap =
    humanoid_robot::PB::sdk_service::navigation::RequestGridMap;
using OccupancyGrid = humanoid_robot::PB::ros2::nav_msgs::OccupancyGrid;
using Goals = humanoid_robot::PB::ros2::nav_msgs::Goals;
using RequestRemainingDistance =
    humanoid_robot::PB::sdk_service::navigation::RequestRemainingDistance;
using ResponseRemainingDistance =
    humanoid_robot::PB::sdk_service::navigation::ResponseRemainingDistance;
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

NavigationResStatus GetCurrentPose(std::unique_ptr<InterfacesClient>& client,
                                   const ReqPoseMsg& request,
                                   Pose& current_pose);

NavigationResStatus GetGridMap2D(std::unique_ptr<InterfacesClient>& client,
                                 const RequestGridMap& request,
                                 OccupancyGrid& occupancy_grid_map);

NavigationResStatus NavigationTo(std::unique_ptr<InterfacesClient>& client,
                                 const Goals& goals,
                                 ResStartNav& res_start_nav);

NavigationResStatus GetRemainingPathDistance(
    std::unique_ptr<InterfacesClient>& client,
    const RequestRemainingDistance& request,
    ResponseRemainingDistance& remaining_distance);

NavigationResStatus CancelNavigationTask(
    std::unique_ptr<InterfacesClient>& client,
    const RequestCancelNavigation& request,
    ResponseCancelNavigation& cancel_status);

NavigationResStatus StartChargingTask(std::unique_ptr<InterfacesClient>& client,
                                      const RequestStartCharging& request,
                                      ResponseStartCharging& charging_status);

NavigationResStatus StopChargingTask(std::unique_ptr<InterfacesClient>& client,
                                     const RequestStopCharging& request,
                                     ResponseStopCharging& charging_status);

}  // namespace navigation_api
}  // namespace robot
}  // namespace clientSDK
}  // namespace humanoid_robot
#endif  // HUMANOID_ROBOT_INTERFACES_NAVIGATIONAPI