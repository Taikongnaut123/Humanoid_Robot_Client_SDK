#ifndef HUMANOID_ROBOT_INTERFACES_CONTROLAPI
#define HUMANOID_ROBOT_INTERFACES_CONTROLAPI

#include "robot/client/interfaces_client.h"
#include "sdk_service/common/service.pb.h"
#include "sdk_service/control/request_emergency_stop.pb.h"
#include "sdk_service/control/response_emergency_stop.pb.h"
#include "sdk_service/control/request_get_joint_info.pb.h"
#include "sdk_service/control/response_get_joint_info.pb.h"
#include "sdk_service/control/request_joint_motion.pb.h"
#include "sdk_service/control/response_joint_motion.pb.h"
#include "sdk_service/control/response_status.pb.h"

namespace humanoid_robot {
namespace konka_sdk {
namespace robot {
namespace control_api {
using RequestEmergencyStop = humanoid_robot::PB::sdk_service::control::RequestEmergencyStop;
using ResponseEmergencyStop = humanoid_robot::PB::sdk_service::control::ResponseEmergencyStop;
using RequestGetJointInfo = humanoid_robot::PB::sdk_service::control::RequestGetJointInfo;
using ResponseGetJointInfo = humanoid_robot::PB::sdk_service::control::ResponseGetJointInfo;
using RequestJointMotion = humanoid_robot::PB::sdk_service::control::RequestJointMotion;
using ResponseJointMotion = humanoid_robot::PB::sdk_service::control::ResponseJointMotion;


using ControlResStatus = humanoid_robot::PB::sdk_service::control::ResponseStatus;
using InterfacesClient = humanoid_robot::konka_sdk::robot::InterfacesClient;

using SendRequest = humanoid_robot::PB::interfaces::SendRequest;
using SendResponse = humanoid_robot::PB::interfaces::SendResponse;
using ControlCommandCode =
    humanoid_robot::PB::sdk_service::common::ControlCommandCode;

ControlResStatus EmergencyStop(std::unique_ptr<InterfacesClient>& client,
                                const RequestEmergencyStop& request_emergency_stop,
                                ResponseEmergencyStop& response_emergency_stop);

ControlResStatus GetJointInfo(std::unique_ptr<InterfacesClient>& client,
                                const RequestGetJointInfo& request_get_joint_info,
                                ResponseGetJointInfo& response_get_joint_info);

ControlResStatus JointMotion(std::unique_ptr<InterfacesClient>& client,
                                const RequestJointMotion& request_joint_motion,
                                ResponseJointMotion& response_joint_motion);

}  // namespace control_api
}  // namespace robot
}  // namespace konka_sdk
}  // namespace humanoid_robot
#endif  // HUMANOID_ROBOT_INTERFACES_CONTROLAPI