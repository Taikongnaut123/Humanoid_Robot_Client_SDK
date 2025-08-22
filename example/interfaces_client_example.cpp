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

using namespace humanoid_robot::PB::interfaces;
using namespace humanoid_robot::PB::common;
using namespace humanoid_robot::clientSDK::robot;

static void print_keyvaluelist(const ::google::protobuf::Map<std::string, humanoid_robot::PB::common::Variant> &kvl)
{
    std::cout << "Response data:" << std::endl;
    for (const auto &entry : kvl)
    {
        const std::string &key = entry.first;
        const auto &variant = entry.second;
        std::cout << "  " << key << " = ";
        if (variant.has_stringvalue())
            std::cout << '"' << variant.stringvalue() << '"';
        else if (variant.has_int32value())
            std::cout << variant.int32value();
        else if (variant.has_doublevalue())
            std::cout << variant.doublevalue();
        else if (variant.has_boolvalue())
            std::cout << (variant.boolvalue() ? "true" : "false");
        else
            std::cout << "(other)";
        std::cout << std::endl;
    }
}

static void TestDetectionService(InterfacesClient &client)
{
    std::cout << "\n--- Testing Detection Service ---" << std::endl;

    try
    {
        SendRequest send_req;
        SendResponse send_resp;

        // Set command ID and input
        auto input_map = send_req.mutable_input()->mutable_keyvaluelist();
        {
            Variant var;
            var.set_int32value(20002); // GET_DETECTION_RESULT
            input_map->insert(std::make_pair(std::string("commandID"), var));
        }
        // Simulated image (read bytes from perception_pipeline_cpp/test/frame.jpg)
        {
            Variant data;
            auto dataMap = data.mutable_dictvalue()->mutable_keyvaluelist();

            const std::string img_path = "/home/ubuntu/zhaokai/vs_workspace/Humanoid-Robot/perception_pipeline_cpp/test/frame.jpg"; // 相对于 build/bin/linux_x64/debug
            std::ifstream ifs(img_path, std::ios::binary);
            if (ifs)
            {
                std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
                {
                    Variant var;
                    var.set_bytevalue(content); // 将图片二进制放入 Variant.byteValue
                    dataMap->insert(std::make_pair(std::string("image"), var));
                }

                {
                    Variant var;
                    var.set_int32value(content.size()); // 将图片大小放入 Variant.int32Value
                    dataMap->insert(std::make_pair(std::string("image_size"), var));
                    std::cout << "Image size: " << content.size() << " bytes" << std::endl;
                }
            }
            else
            {
                // 回退到占位符字符串，且打印警告
                Variant var;
                var.set_stringvalue("test_image_data_detection");
                dataMap->insert(std::make_pair(std::string("image_bytes"), var));
                std::cerr << "Warning: failed to open image: " << img_path << " - using placeholder string" << std::endl;
            }

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
            std::cout << "[✓] Detection response successful" << std::endl;
            print_keyvaluelist(send_resp.output().keyvaluelist());
        }
        else
        {
            std::cerr << "[✗] No response received" << std::endl;
        }

        stream->WritesDone();
        auto finish_status = stream->Finish();
        std::cout << "Stream finished: " << (finish_status.ok() ? "ok" : "error") << std::endl;
        // context will be destroyed when unique_ptr goes out of scope
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception in detection test: " << e.what() << std::endl;
    }
}

static void TestSegmentationService(InterfacesClient &client)
{
    std::cout << "\n--- Testing Segmentation Service ---" << std::endl;
    // TODO: Implement segmentation service test
    try
    {
        SendRequest send_req;
        SendResponse send_resp;

        // Set command ID and input
        auto input_map = send_req.mutable_input()->mutable_keyvaluelist();
        {
            Variant var;
            var.set_int32value(20003); // GET_SEGMENTATION_RESULT
            input_map->insert(std::make_pair(std::string("commandID"), var));
        }
        // Simulated image (read bytes from perception_pipeline_cpp/test/frame.jpg)
        {
            Variant data;
            auto dataMap = data.mutable_dictvalue()->mutable_keyvaluelist();

            const std::string img_path = "/home/ubuntu/zhaokai/vs_workspace/Humanoid-Robot/perception_pipeline_cpp/test/frame.jpg"; // 相对于 build/bin/linux_x64/debug
            std::ifstream ifs(img_path, std::ios::binary);
            if (ifs)
            {
                std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
                {
                    Variant var;
                    var.set_bytevalue(content); // 将图片二进制放入 Variant.byteValue
                    dataMap->insert(std::make_pair(std::string("image"), var));
                }

                {
                    Variant var;
                    var.set_int32value(content.size()); // 将图片大小放入 Variant.int32Value
                    dataMap->insert(std::make_pair(std::string("image_size"), var));
                    std::cout << "Image size: " << content.size() << " bytes" << std::endl;
                }
            }
            else
            {
                // 回退到占位符字符串，且打印警告
                Variant var;
                var.set_stringvalue("test_image_data_detection");
                dataMap->insert(std::make_pair(std::string("image_bytes"), var));
                std::cerr << "Warning: failed to open image: " << img_path << " - using placeholder string" << std::endl;
            }

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
            std::cerr << "Failed to create Segmentation stream: " << status.message() << std::endl;
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
            std::cout << "[✓] Segmentation response successful" << std::endl;
            print_keyvaluelist(send_resp.output().keyvaluelist());
        }
        else
        {
            std::cerr << "[✗] No Segmentation response received" << std::endl;
        }

        stream->WritesDone();
        auto finish_status = stream->Finish();
        std::cout << "Segmentation Stream finished: " << (finish_status.ok() ? "ok" : "error") << std::endl;
        // context will be destroyed when unique_ptr goes out of scope
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception in segmentation test: " << e.what() << std::endl;
    }
}

static void TestPerceptionService(InterfacesClient &client)
{
    std::cout << "\n--- Testing Perception Service ---" << std::endl;
    // TODO: Implement perception service test
    try
    {
        SendRequest send_req;
        SendResponse send_resp;

        // Set command ID and input
        auto input_map = send_req.mutable_input()->mutable_keyvaluelist();
        {
            Variant var;
            var.set_int32value(20001); // GET_PERCEPTION_RESULT
            input_map->insert(std::make_pair(std::string("commandID"), var));
        }
        // Simulated image (read bytes from perception_pipeline_cpp/test/frame.jpg)
        {
            Variant data;
            auto dataMap = data.mutable_dictvalue()->mutable_keyvaluelist();

            const std::string img_path = "/home/ubuntu/zhaokai/vs_workspace/Humanoid-Robot/perception_pipeline_cpp/test/frame.jpg"; // 相对于 build/bin/linux_x64/debug
            std::ifstream ifs(img_path, std::ios::binary);
            if (ifs)
            {
                std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
                {
                    Variant var;
                    var.set_bytevalue(content); // 将图片二进制放入 Variant.byteValue
                    dataMap->insert(std::make_pair(std::string("image"), var));
                }

                {
                    Variant var;
                    var.set_int32value(content.size()); // 将图片大小放入 Variant.int32Value
                    dataMap->insert(std::make_pair(std::string("image_size"), var));
                    std::cout << "Image size: " << content.size() << " bytes" << std::endl;
                }
            }
            else
            {
                // 回退到占位符字符串，且打印警告
                Variant var;
                var.set_stringvalue("test_image_data_detection");
                dataMap->insert(std::make_pair(std::string("image_bytes"), var));
                std::cerr << "Warning: failed to open image: " << img_path << " - using placeholder string" << std::endl;
            }

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
            std::cout << "[✓] Perception response successful" << std::endl;
            print_keyvaluelist(send_resp.output().keyvaluelist());
        }
        else
        {
            std::cerr << "[✗] No Perception response received" << std::endl;
        }

        stream->WritesDone();
        auto finish_status = stream->Finish();
        std::cout << "Perception Stream finished: " << (finish_status.ok() ? "ok" : "error") << std::endl;
        // context will be destroyed when unique_ptr goes out of scope
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception in perception test: " << e.what() << std::endl;
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

        TestDetectionService(*client);
        TestSegmentationService(*client);
        TestPerceptionService(*client);

        std::cout << "\n=== Simple test completed ===" << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fatal exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
