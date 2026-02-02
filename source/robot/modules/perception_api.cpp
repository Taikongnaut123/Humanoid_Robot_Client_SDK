#include "robot/modules/perception/perception_api.h"

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
namespace perception_api {

using Variant = humanoid_robot::PB::common::Variant;
using PerceptionResStatus = humanoid_robot::PB::sdk_service::perception::ResponseStatus;

PerceptionResStatus Detection(std::unique_ptr<InterfacesClient>& client,
                                const RequestDetection& request_detection,
                                ResponseDetection& response_detection)
{
    
    PerceptionResStatus res_status = PerceptionResStatus::ERROR_DATA_GET_FAILED;
    SendRequest send_req;
    SendResponse send_resp;

    try {
        auto input_map = send_req.mutable_input()->mutable_keyvaluelist();
        
        {
            Variant command_id;
            command_id.set_int32value(PerceptionCommandCode::kDetection);
            input_map->insert(std::make_pair(std::string("command_id"), command_id));
        }

        {
            Variant request_dict;
            auto request_dict_map =
                        request_dict.mutable_dictvalue()->mutable_keyvaluelist();
            Variant request_params;
            std::string serialize_data;
            
            auto serialize_status = request_detection.SerializeToString(&serialize_data);
            
            if (!serialize_status) {
                std::cerr << "Failed to serialize request_detection." << std::endl;
                return PerceptionResStatus::ERROR_PARSE_FAILED;
            }
            
            request_params.set_bytevalue(serialize_data);
            request_dict_map->insert(
                std::make_pair(std::string("request_detection"), request_params));
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
            std::cerr << "Failed to write Detection request" << std::endl;
            stream->WritesDone();
            stream->Finish();
            return res_status;
        }

        if (stream->Read(&send_resp)) {
            std::cout << "[✓] Detection response received" << std::endl;
            std::cout << "[✓] Response code: " << send_resp.ret().code() << std::endl;
            std::cout << "[✓] Response message: " << send_resp.ret().message() << std::endl;

            auto response_status = send_resp.ret();
            try {
                res_status = static_cast<PerceptionResStatus>(std::stoi(response_status.code()));
            } catch (const std::invalid_argument& e) {
                std::cerr << "Invalid response code: " << response_status.code() << std::endl;
                return PerceptionResStatus::ERROR_UNKNOWN_SERVICE;
            }

            auto response_output = send_resp.output();
            auto data_it = response_output.keyvaluelist().find("data");
            if (data_it == response_output.keyvaluelist().end()) {
                std::cerr << "'data' field not found in Detection response" << std::endl;
                return res_status;
            }

            const Variant& data_var = data_it->second;
            auto unserialize_status =
                response_detection.ParseFromString(data_var.bytevalue());
            if (!unserialize_status) {
                std::cerr << "Failed to unserialize response_detection" << std::endl;
                return PerceptionResStatus::ERROR_PARSE_FAILED;
            }
            
        } else {
            std::cerr << "No Detection response received from server" << std::endl;
            return res_status;
        }

        stream->WritesDone();
        auto finish_status = stream->Finish();
        std::cout << "[✓] Detection stream finished: " 
                  << (finish_status.ok() ? "success" : "failed") << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Exception in Detection: " << e.what() << std::endl;
        res_status = PerceptionResStatus::ERROR_UNKNOWN_SERVICE;
    }
    return res_status;
}

PerceptionResStatus Division(std::unique_ptr<InterfacesClient>& client,
                                const RequestDivision& request_division,
                                ResponseDivision& response_division)
{    
    PerceptionResStatus res_status = PerceptionResStatus::ERROR_DATA_GET_FAILED;
    SendRequest send_req;
    SendResponse send_resp;

    try {
        auto input_map = send_req.mutable_input()->mutable_keyvaluelist();
        
        {
            Variant command_id;
            command_id.set_int32value(PerceptionCommandCode::kDivision);
            input_map->insert({"command_id", command_id});
        }

        {
            Variant request_dict;
            auto request_dict_map =
                        request_dict.mutable_dictvalue()->mutable_keyvaluelist();
            Variant request_params;
            std::string serialize_data;
            auto serialize_status = request_division.SerializeToString(&serialize_data);
            if (!serialize_status) {
                std::cerr << "Serialize request_division failed." << std::endl;
                return PerceptionResStatus::ERROR_PARSE_FAILED;
            }
            request_params.set_bytevalue(serialize_data);
            request_dict_map->insert(
                std::make_pair(std::string("request_division"), request_params));
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
            std::cerr << "Write Division request failed" << std::endl;
            stream->WritesDone();
            stream->Finish();
            return res_status;
        }

        if (stream->Read(&send_resp)) {
            std::cout << "[✓] Division response received" << std::endl;
            res_status = static_cast<PerceptionResStatus>(std::stoi(send_resp.ret().code()));

            auto data_it = send_resp.output().keyvaluelist().find("data");
            if (data_it != send_resp.output().keyvaluelist().end()) {
                const Variant& data_var = data_it->second;
                auto unserialize_status =
                response_division.ParseFromString(data_var.bytevalue());
                if (!unserialize_status) {
                    std::cerr << "Failed to unserialize response_division" << std::endl;
                    return PerceptionResStatus::ERROR_PARSE_FAILED;
                }
            }
        } else {
            std::cerr << "No Division response received" << std::endl;
            return res_status;
        }

        stream->WritesDone();
        stream->Finish();

    } catch (const std::exception& e) {
        std::cerr << "Exception in Division: " << e.what() << std::endl;
    }

    return res_status;
}

PerceptionResStatus Perception(std::unique_ptr<InterfacesClient>& client,
                                const RequestPerception& request_perception,
                                ResponsePerception& response_perception)
{    
    PerceptionResStatus res_status = PerceptionResStatus::ERROR_DATA_GET_FAILED;
    SendRequest send_req;
    SendResponse send_resp;

    try {
        auto input_map = send_req.mutable_input()->mutable_keyvaluelist();
        
        {
            Variant command_id;
            command_id.set_int32value(PerceptionCommandCode::kPerception);
            input_map->insert({"command_id", command_id});
        }

        {
            Variant request_dict;
            auto request_dict_map =
                        request_dict.mutable_dictvalue()->mutable_keyvaluelist();
            Variant request_params;
            std::string serialize_data;
            auto serialize_status = request_perception.SerializeToString(&serialize_data);
            if (!serialize_status) {
                std::cerr << "Serialize request_perception failed." << std::endl;
                return PerceptionResStatus::ERROR_PARSE_FAILED;
            }
            request_params.set_bytevalue(serialize_data);
            request_dict_map->insert(
                std::make_pair(std::string("request_perception"), request_params));
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
            std::cerr << "Write Perception request failed" << std::endl;
            stream->WritesDone();
            stream->Finish();
            return res_status;
        }

        if (stream->Read(&send_resp)) {
            std::cout << "[✓] Perception response received" << std::endl;
            res_status = static_cast<PerceptionResStatus>(std::stoi(send_resp.ret().code()));

            auto data_it = send_resp.output().keyvaluelist().find("data");
            if (data_it != send_resp.output().keyvaluelist().end()) {
                const Variant& data_var = data_it->second;
                auto unserialize_status = 
                response_perception.ParseFromString(data_var.bytevalue());
                if (!unserialize_status) {
                    std::cerr << "Failed to unserialize response_perception" << std::endl;
                    return PerceptionResStatus::ERROR_PARSE_FAILED;
                }
                }
            
        } else {
            std::cerr << "No Perception response received" << std::endl;
            return res_status;
        }

        stream->WritesDone();
        stream->Finish();

    } catch (const std::exception& e) {
        std::cerr << "Exception in Perception: " << e.what() << std::endl;
    }

    return res_status;
}

}  // namespace perception_api
}  // namespace robot
}  // namespace clientSDK
}  // namespace humanoid_robot
