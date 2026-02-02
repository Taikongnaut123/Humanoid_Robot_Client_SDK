#include "robot/modules/control/control_api.h"

#include <iostream>
#include <string>
#include <stdexcept>

#include "common/variant.pb.h"
#include "grpcpp/support/sync_stream.h"
#include "interfaces/interfaces_request_response.pb.h"
#include "robot/client/interfaces_client.h"
#include "sdk_service/common/service.pb.h"

#include "robot/common/json_convert_util.hpp"

namespace humanoid_robot {
namespace clientSDK {
namespace robot {
namespace control_api {

using Variant = humanoid_robot::PB::common::Variant;
using ControlResStatus = humanoid_robot::PB::sdk_service::control::ResponceStatus;

ControlResStatus EmergencyStop(
    std::unique_ptr<InterfacesClient>& client,
    const RequestEmergencyStop& request_emergency_stop,
    ResponceEmergencyStop& response_emergency_stop) {
    
    ControlResStatus res_status = ControlResStatus::ERROR_DATA_GET_FAILED;
    SendRequest send_req;
    SendResponse send_resp;

    try {
        auto input_map = send_req.mutable_input()->mutable_keyvaluelist();
        
        {
            Variant command_id;
            command_id.set_int32value(ControlCommandCode::kEmergencyStop);
            input_map->insert(std::make_pair(std::string("command_id"), command_id));
        }

        {
            Variant request_dict;
            auto request_dict_map =
                        request_dict.mutable_dictvalue()->mutable_keyvaluelist();
            Variant request_params;
            std::string serialize_data;
            
            auto serialize_status = request_emergency_stop.SerializeToString(&serialize_data);
            
            if (!serialize_status) {
                std::cerr << "Failed to serialize request_emergency_stop." << std::endl;
                return ControlResStatus::ERROR_PARSE_FAILED;
            }
            
            request_params.set_bytevalue(serialize_data);
            request_dict_map->insert(
                std::make_pair(std::string("request_emergency_stop"), request_params));
            input_map->insert(std::make_pair(std::string("data"), request_dict));
        }

        std::unique_ptr<::grpc::ClientReaderWriter<SendRequest, SendResponse>> stream;
        std::unique_ptr<grpc::ClientContext> context;
        auto send_status = client->Send(stream, context, 10000);
        
        if (!send_status) {
            std::cerr << "Failed to create gRPC stream: " 
                      << send_status.message() << std::endl;
            return res_status;
        }

        if (!stream->Write(send_req)) {
            std::cerr << "Failed to write EmergencyStop request" << std::endl;
            stream->WritesDone();
            stream->Finish();
            return res_status;
        }

        if (stream->Read(&send_resp)) {
            std::cout << "[✓] EmergencyStop response received" << std::endl;
            std::cout << "[✓] Response code: " << send_resp.ret().code() << std::endl;
            std::cout << "[✓] Response message: " << send_resp.ret().message() << std::endl;

            auto response_status = send_resp.ret();
            try {
                res_status = static_cast<ControlResStatus>(std::stoi(response_status.code()));
            } catch (const std::invalid_argument& e) {
                std::cerr << "Invalid response code: " << response_status.code() << std::endl;
                return ControlResStatus::ERROR_UNKNOWN_SERVICE;
            }

            auto response_output = send_resp.output();
            auto data_it = response_output.keyvaluelist().find("data");
            if (data_it == response_output.keyvaluelist().end()) {
                std::cerr << "'data' field not found in EmergencyStop response" << std::endl;
                return res_status;
            }

            const Variant& data_var = data_it->second;
            auto unserialize_status =
                response_emergency_stop.ParseFromString(data_var.bytevalue());
            if (!unserialize_status) {
                std::cerr << "Failed to unserialize response_emergency_stop" << std::endl;
                return ControlResStatus::ERROR_PARSE_FAILED;
            }
            
        } else {
            std::cerr << "No EmergencyStop response received from server" << std::endl;
            return res_status;
        }

        stream->WritesDone();
        auto finish_status = stream->Finish();
        std::cout << "[✓] EmergencyStop stream finished: " 
                  << (finish_status.ok() ? "success" : "failed") << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Exception in EmergencyStop: " << e.what() << std::endl;
        res_status = ControlResStatus::ERROR_UNKNOWN_SERVICE;
    }
    return res_status;
}

ControlResStatus GetJointInfo(
    std::unique_ptr<InterfacesClient>& client,
    const RequestGetJointInfo& request_get_joint_info,
    ResponceGetJointInfo& response_get_joint_info) {
    
    ControlResStatus res_status = ControlResStatus::ERROR_DATA_GET_FAILED;
    SendRequest send_req;
    SendResponse send_resp;

    try {
        auto input_map = send_req.mutable_input()->mutable_keyvaluelist();
        
        {
            Variant command_id;
            command_id.set_int32value(ControlCommandCode::kGetJointInfo);
            input_map->insert({"command_id", command_id});
        }

        {
            Variant request_dict;
            auto request_dict_map =
                        request_dict.mutable_dictvalue()->mutable_keyvaluelist();
            Variant request_params;
            std::string serialize_data;
            auto serialize_status = request_get_joint_info.SerializeToString(&serialize_data);
            if (!serialize_status) {
                std::cerr << "Serialize request_get_joint_info failed." << std::endl;
                return ControlResStatus::ERROR_PARSE_FAILED;
            }
            request_params.set_bytevalue(serialize_data);
            request_dict_map->insert(
                std::make_pair(std::string("request_get_joint_info"), request_params));
            input_map->insert(std::make_pair(std::string("data"), request_dict));
        }

        std::unique_ptr<grpc::ClientReaderWriter<SendRequest, SendResponse>> stream;
        std::unique_ptr<grpc::ClientContext> context;
        auto send_status = client->Send(stream, context, 10000);
        if (!send_status) {
            std::cerr << "Create stream failed: " << send_status.message() << std::endl;
            return res_status;
        }

        if (!stream->Write(send_req)) {
            std::cerr << "Write GetJointInfo request failed" << std::endl;
            stream->WritesDone();
            stream->Finish();
            return res_status;
        }

        if (stream->Read(&send_resp)) {
            std::cout << "[✓] GetJointInfo response received" << std::endl;
            res_status = static_cast<ControlResStatus>(std::stoi(send_resp.ret().code()));

            auto data_it = send_resp.output().keyvaluelist().find("data");
            if (data_it != send_resp.output().keyvaluelist().end()) {
                const Variant& data_var = data_it->second;
                auto unserialize_status =
                response_get_joint_info.ParseFromString(data_var.bytevalue());
                if (!unserialize_status) {
                    std::cerr << "Failed to unserialize response_get_joint_info" << std::endl;
                    return ControlResStatus::ERROR_PARSE_FAILED;
                }
            }
        } else {
            std::cerr << "No GetJointInfo response received" << std::endl;
            return res_status;
        }

        stream->WritesDone();
        stream->Finish();

    } catch (const std::exception& e) {
        std::cerr << "Exception in GetJointInfo: " << e.what() << std::endl;
    }

    return res_status;
}

ControlResStatus JointMotion(
    std::unique_ptr<InterfacesClient>& client,
    const RequestJointMotion& request_joint_motion,
    ResponceJointMotion& response_joint_motion) {
    
    ControlResStatus res_status = ControlResStatus::ERROR_DATA_GET_FAILED;
    SendRequest send_req;
    SendResponse send_resp;

    try {
        auto input_map = send_req.mutable_input()->mutable_keyvaluelist();
        
        {
            Variant command_id;
            command_id.set_int32value(ControlCommandCode::kJointMotion);
            input_map->insert({"command_id", command_id});
        }

        {
            Variant request_dict;
            auto request_dict_map =
                        request_dict.mutable_dictvalue()->mutable_keyvaluelist();
            Variant request_params;
            std::string serialize_data;
            auto serialize_status = request_joint_motion.SerializeToString(&serialize_data);
            if (!serialize_status) {
                std::cerr << "Serialize request_joint_motion failed." << std::endl;
                return ControlResStatus::ERROR_PARSE_FAILED;
            }
            request_params.set_bytevalue(serialize_data);
            request_dict_map->insert(
                std::make_pair(std::string("request_joint_motion"), request_params));
            input_map->insert(std::make_pair(std::string("data"), request_dict));
        }

        std::unique_ptr<grpc::ClientReaderWriter<SendRequest, SendResponse>> stream;
        std::unique_ptr<grpc::ClientContext> context;
        auto send_status = client->Send(stream, context, 10000);
        if (!send_status) {
            std::cerr << "Create stream failed: " << send_status.message() << std::endl;
            return res_status;
        }

        if (!stream->Write(send_req)) {
            std::cerr << "Write JointMotion request failed" << std::endl;
            stream->WritesDone();
            stream->Finish();
            return res_status;
        }

        if (stream->Read(&send_resp)) {
            std::cout << "[✓] JointMotion response received" << std::endl;
            res_status = static_cast<ControlResStatus>(std::stoi(send_resp.ret().code()));

            auto data_it = send_resp.output().keyvaluelist().find("data");
            if (data_it != send_resp.output().keyvaluelist().end()) {
                const Variant& data_var = data_it->second;
                auto unserialize_status = 
                response_joint_motion.ParseFromString(data_var.bytevalue());
                if (!unserialize_status) {
                    std::cerr << "Failed to unserialize response_joint_motion" << std::endl;
                    return ControlResStatus::ERROR_PARSE_FAILED;
                }
                }
            
        } else {
            std::cerr << "No JointMotion response received" << std::endl;
            return res_status;
        }

        stream->WritesDone();
        stream->Finish();

    } catch (const std::exception& e) {
        std::cerr << "Exception in JointMotion: " << e.what() << std::endl;
    }

    return res_status;
}

}  // namespace control_api
}  // namespace robot
}  // namespace clientSDK
}  // namespace humanoid_robot
