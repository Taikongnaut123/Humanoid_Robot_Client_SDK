#ifndef HUMANOID_ROBOT_INTERFACES_CONTROLAPI
#define HUMANOID_ROBOT_INTERFACES_CONTROLAPI

#include "robot/client/interfaces_client.h"
#include "sdk_service/common/service.pb.h"
#include "sdk_service/control/request_emergency_stop.pb.h"
#include "sdk_service/control/responce_emergency_stop.pb.h"
#include "sdk_service/control/request_get_joint_info.pb.h"
#include "sdk_service/control/responce_get_joint_info.pb.h"
#include "sdk_service/control/request_joint_motion.pb.h"
#include "sdk_service/control/responce_joint_motion.pb.h"
#include "sdk_service/control/responce_status.pb.h"

namespace humanoid_robot {
namespace clientSDK {
namespace robot {
namespace control_api {
using RequestEmergencyStop = humanoid_robot::PB::sdk_service::control::RequestEmergencyStop;
using ResponceEmergencyStop = humanoid_robot::PB::sdk_service::control::ResponceEmergencyStop;
using RequestGetJointInfo = humanoid_robot::PB::sdk_service::control::RequestGetJointInfo;
using ResponceGetJointInfo = humanoid_robot::PB::sdk_service::control::ResponceGetJointInfo;
using RequestJointMotion = humanoid_robot::PB::sdk_service::control::RequestJointMotion;
using ResponceJointMotion = humanoid_robot::PB::sdk_service::control::ResponceJointMotion;


using ControlResStatus = humanoid_robot::PB::sdk_service::control::ResponceStatus;
using InterfacesClient = humanoid_robot::clientSDK::robot::InterfacesClient;

using SendRequest = humanoid_robot::PB::interfaces::SendRequest;
using SendResponse = humanoid_robot::PB::interfaces::SendResponse;
using ControlCommandCode =
    humanoid_robot::PB::sdk_service::common::ControlCommandCode;

ControlResStatus EmergencyStop(std::unique_ptr<InterfacesClient>& client,
                                const RequestEmergencyStop& request_emergency_stop,
                                ResponceEmergencyStop& responce_emergency_stop);

ControlResStatus GetJointInfo(std::unique_ptr<InterfacesClient>& client,
                                const RequestGetJointInfo& request_get_joint_info,
                                ResponceGetJointInfo& response_get_joint_info);

ControlResStatus JointMotion(std::unique_ptr<InterfacesClient>& client,
                                const RequestJointMotion& request_joint_motion,
                                ResponceJointMotion& response_joint_motion);

}  // namespace control_api
}  // namespace robot
}  // namespace clientSDK
}  // namespace humanoid_robot
#endif  // HUMANOID_ROBOT_INTERFACES_CONTROLAPI