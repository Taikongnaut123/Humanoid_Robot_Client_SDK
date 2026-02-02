#ifndef HUMANOID_ROBOT_INTERFACES_PERCEPTIONAPI
#define HUMANOID_ROBOT_INTERFACES_PERCEPTIONAPI

#include "robot/client/interfaces_client.h"
#include "sdk_service/common/service.pb.h"
#include "sdk_service/perception/request_detection.pb.h"
#include "sdk_service/perception/response_detection.pb.h"
#include "sdk_service/perception/request_division.pb.h"
#include "sdk_service/perception/response_division.pb.h"
#include "sdk_service/perception/request_perception.pb.h"
#include "sdk_service/perception/response_perception.pb.h"
#include "sdk_service/perception/perception_service.pb.h"
#include "sdk_service/perception/response_status.pb.h"

namespace humanoid_robot {
namespace clientSDK {
namespace robot {
namespace perception_api {
using RequestDetection = humanoid_robot::PB::sdk_service::perception::RequestDetection;
using ResponseDetection = humanoid_robot::PB::sdk_service::perception::ResponseDetection;
using RequestDivision = humanoid_robot::PB::sdk_service::perception::RequestDivision;
using ResponseDivision = humanoid_robot::PB::sdk_service::perception::ResponseDivision;
using RequestPerception = humanoid_robot::PB::sdk_service::perception::RequestPerception;
using ResponsePerception = humanoid_robot::PB::sdk_service::perception::ResponsePerception;


using PerceptionResStatus = humanoid_robot::PB::sdk_service::perception::ResponseStatus;
using InterfacesClient = humanoid_robot::clientSDK::robot::InterfacesClient;

using SendRequest = humanoid_robot::PB::interfaces::SendRequest;
using SendResponse = humanoid_robot::PB::interfaces::SendResponse;
using PerceptionCommandCode =
    humanoid_robot::PB::sdk_service::common::PerceptionCommandCode;

PerceptionResStatus Detection(std::unique_ptr<InterfacesClient>& client,
                                const RequestDetection& request_detection,
                                ResponseDetection& response_detection);

PerceptionResStatus Division(std::unique_ptr<InterfacesClient>& client,
                                const RequestDivision& request_division,
                                ResponseDivision& response_division);

PerceptionResStatus Perception(std::unique_ptr<InterfacesClient>& client,
                                const RequestPerception& request_perception,
                                ResponsePerception& response_perception);

}  // namespace perception_api
}  // namespace robot
}  // namespace clientSDK
}  // namespace humanoid_robot
#endif  // HUMANOID_ROBOT_INTERFACES_PERCEPTIONAPI