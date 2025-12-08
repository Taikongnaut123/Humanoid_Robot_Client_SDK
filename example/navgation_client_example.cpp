/**
 * Copyright (c) 2025 Humanoid Robot, Inc. All rights reserved.
 *
 * Example usage of InterfacesClient
 */

#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <fstream>

#include "robot/client/interfaces_client.h"
#include "interfaces/interfaces_request_response.pb.h"
#include "interfaces/interfaces_request_response.grpc.pb.h"
#include "common/variant.pb.h"
#include "common/service.pb.h"

#include "loc_msg/msg/req_pose_msg.hpp"
#include "loc_msg/msg/res_start_nav.hpp"
#include "loc_msg/msg/res_status.hpp"
#include "rclcpp/serialization.hpp"
#include "rclcpp/serialized_message.hpp"
#include "geometry_msgs/msg/pose.hpp"

#include "common/RosMessageSerializer.h"
#include "absl/strings/escaping.h"

// #include "common/pose.pb.h"
// #include "navigation/navigation_request_response.pb.h"

using namespace humanoid_robot::PB::interfaces;
using namespace humanoid_robot::PB::common;
using namespace humanoid_robot::clientSDK::robot;
using namespace humanoid_robot::framework::communication;

// rclcpp::Serialization<loc_msg::msg::ReqPoseMsg> req_pose_serializer;
static RosMessageSerializer<geometry_msgs::msg::Pose> geometry_pose_serializer;

static void print_keyvaluelist(const ::google::protobuf::Map<std::string, humanoid_robot::PB::common::Variant> &kvl)
{
    std::cout << "Response data:" << std::endl;
    for (const auto &entry : kvl)
    {
        const std::string &key = entry.first;
        const auto &variant = entry.second;
        std::cout << "  " << key << " = ";
        if (variant.has_stringvalue())
        {
            // std::cout <<  variant.stringvalue();
            if (key == "pose")
            {
                geometry_msgs::msg::Pose pose;
                std::string decoded;
                absl::Base64Unescape(variant.stringvalue(), &decoded);
                geometry_pose_serializer.DeserializeFromString(decoded, pose);

                // geometry_pose_serializer.DeserializeFromString(variant.stringvalue(), pose);
                std::cout << "posiytion: " << pose.position.x << ", " << pose.position.y << ", " << pose.position.z << std::endl;
                std::cout << "orientation: " << pose.orientation.x << ", " << pose.orientation.y << ", " << pose.orientation.z << ", " << pose.orientation.w << std::endl;
            }else
            {
                std::cout <<  variant.stringvalue();
            }
        }
        else if (variant.has_int32value())
            std::cout << variant.int32value();
        else if (variant.has_uint32value())
            std::cout << variant.uint32value();
        else if (variant.has_doublevalue())
            std::cout << variant.doublevalue();
        else if (variant.has_boolvalue())
            std::cout << (variant.boolvalue() ? "true" : "false");

        else
            std::cout << "(other)";
        std::cout << std::endl;
    }
}

static void TestNavigationService(InterfacesClient &client)
{
    std::cout << "\n--- Testing Navigation Service ---" << std::endl;
    // TODO: Implement navigation service test
    try
    {
        SendRequest send_req;
        SendResponse send_resp;

        // Set command ID and input
        auto input_map = send_req.mutable_input()->mutable_keyvaluelist();
        {
            Variant var;
            var.set_int32value(CommandCode::GET_CURRENT_POSE); // GET_CURRENT_POSE
            input_map->insert(std::make_pair(std::string("commandID"), var));
        }
        // Simulated data (empty for GET_CURRENT_POSE)
        {
            Variant data;
            auto dataMap = data.mutable_dictvalue()->mutable_keyvaluelist();

            Variant frame_id;
            frame_id.set_stringvalue("map");
            dataMap->insert(std::make_pair(std::string("frame_id"), frame_id));

            Variant child_frame_id;
            child_frame_id.set_stringvalue("base_link");
            dataMap->insert(std::make_pair(std::string("child_frame_id"), child_frame_id));

            input_map->insert(std::make_pair(std::string("data"), data));
        }
        // params
        auto params_map = send_req.mutable_params()->mutable_keyvaluelist();
        {
            Variant var;
            var.set_doublevalue(0.5);
            params_map->insert(std::make_pair(std::string("confidence_threshold"), var));
        }

        std::unique_ptr<::grpc::ClientReaderWriter<SendRequest, SendResponse>> stream;
        std::unique_ptr<grpc::ClientContext> context;
        auto status = client.Send(stream, context, 10000);
        if (!status)
        {
            std::cerr << "Failed to create stream: " << status.message() << std::endl;
            return;
        }

        if (!stream->Write(send_req))
        {
            std::cerr << "Failed to write request" << std::endl;
            stream->WritesDone();
            stream->Finish();
            return;
        }

        if (stream->Read(&send_resp))
        {
            std::cout << "[✓] Navigation response successful" << std::endl;
            std::cout << "[✓] Navigation response ret: " << send_resp.ret().code() << std::endl;
            std::cout << "[✓] Navigation response ret: " << send_resp.ret().message() << std::endl;
            print_keyvaluelist(send_resp.output().keyvaluelist());
        }
        else
        {
            std::cerr << "[✗] No Navigation response received" << std::endl;
        }

        stream->WritesDone();
        auto finish_status = stream->Finish();
        std::cout << "Navigation Stream finished: " << (finish_status.ok() ? "ok" : "error") << std::endl;
        // context will be destroyed when unique_ptr goes out of scope
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception in navigation test: " << e.what() << std::endl;
    }
}

int main()
{
    std::cout << "InterfacesClient Example (simplified)" << std::endl;

    try
    {
        std::unique_ptr<InterfacesClient> client = std::make_unique<InterfacesClient>();
        auto status = client->Connect("localhost:50051");
        if (!status)
        {
            std::cerr << "Failed to connect to Interfaces-Server: " << status.message() << std::endl;
            return 1;
        }
        std::cout << "[✓] Connected to Interfaces-Server at localhost:50051" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        TestNavigationService(*client);

        std::cout << "\n=== Simple test completed ===" << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fatal exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
