/**
 * Copyright (c) 2025 Humanoid Robot, Inc. All rights reserved.
 *
 * Example usage of InterfacesClient
 */

#include <iostream>
#include <memory>
#include <thread>
#include <chrono>

#include "robot/client/interfaces_client.h"
#include "interfaces/interfaces_request_response.pb.h"
#include "printUtil.h" // Custom utility for printing key-value lists
#include "interfaces/interfaces_callback.pb.h"
#include "robot/client/client_callback_server.h"
#include "Log/wlog.hpp"

using namespace humanoid_robot::clientSDK::robot;
using namespace humanoid_robot::clientSDK::common;
using namespace humanoid_robot::PB::common;
using namespace humanoid_robot::PB::interfaces;
using namespace humanoid_robot::utils::PB;

std::chrono::system_clock::time_point GetDeadline(int64_t timeout_ms)
{
    return std::chrono::system_clock::now() + std::chrono::milliseconds(timeout_ms);
}

void PrintStatus(const Status &status, const std::string &operation)
{
    if (status)
    {
        std::cout << "[✓] " << operation << " succeeded" << std::endl;
    }
    else
    {
        std::cout << "[✗] " << operation << " failed: " << status.message() << std::endl;
    }
}

// Example: Synchronous operations
void ExampleSyncOperations(InterfacesClient &client)
{
    std::cout << "\n=== Synchronous Operations Example ===" << std::endl;

    // 2. Send Resource
    SendRequest send_req;

    // Set request data
    auto map = send_req.mutable_input()->mutable_keyvaluelist();
    {
        Variant var;
        var.set_stringvalue("example_resource");
        map->insert(std::make_pair("name", var));
    }
    {
        Variant var;
        var.set_int32value(42);
        map->insert(std::make_pair("value", var));
    }

    auto paramsMap = send_req.mutable_params()->mutable_keyvaluelist();
    {
        Variant var;
        var.set_stringvalue("example_param");
        paramsMap->insert(std::make_pair("param1", var));
    }
    {
        Variant var;
        var.set_int32value(30);
        paramsMap->insert(std::make_pair("timeout", var));
    }
    {
        Variant var;
        var.set_stringvalue("example-001");
        paramsMap->insert(std::make_pair("correlationid", var));
    }
    std::unique_ptr<::grpc::ClientReaderWriter<::humanoid_robot::PB::interfaces::SendRequest, ::humanoid_robot::PB::interfaces::SendResponse>> readWriter;

    auto status = client.Send(readWriter, 5000);
    auto ret = readWriter->Write(send_req);
    PrintStatus(status, "Send Resource");
    SendResponse send_resp;
    ret = readWriter->Read(&send_resp);
    if (!ret)
    {
        std::cerr << "Failed to read response: " << readWriter->Finish().error_message() << std::endl;
        return;
    }
    if (status)
    {
        print_keyvaluelist(send_resp.output().keyvaluelist());
    }

    // 3. Query Resources
    QueryRequest query_req;
    auto input_map = query_req.mutable_input()->mutable_keyvaluelist();
    {
        Variant var;
        var.set_stringvalue("find_resources");
        input_map->insert(std::make_pair("queryid", var));
    }
    {
        Variant var;
        var.set_int32value(10);
        input_map->insert(std::make_pair("limit", var));
    }
    {
        Variant var;
        var.set_int32value(0);
        input_map->insert(std::make_pair("offset", var));
    }

    QueryResponse query_resp;
    status = client.Query(query_req, query_resp);
    PrintStatus(status, "Query Resources");

    if (status)
    {
        print_keyvaluelist(query_resp.output().keyvaluelist());
    }
}

// // Example: Asynchronous operations
// void ExampleAsyncOperations(InterfacesClient &client)
// {
//     std::cout << "\n=== Asynchronous Operations Example ===" << std::endl;

//     // Async Send with future
//     SendRequest send_req;

//     // Set request data
//     auto map = send_req.mutable_input()->mutable_keyvaluelist();
//     {
//         Variant var;
//         var.set_stringvalue("example_resource");
//         map->insert(std::make_pair("name", var));
//     }
//     {
//         Variant var;
//         var.set_int32value(42);
//         map->insert(std::make_pair("value", var));
//     }

//     auto paramsMap = send_req.mutable_params()->mutable_keyvaluelist();
//     {
//         Variant var;
//         var.set_stringvalue("example_param");
//         paramsMap->insert(std::make_pair("param1", var));
//     }
//     {
//         Variant var;
//         var.set_int32value(30);
//         paramsMap->insert(std::make_pair("timeout", var));
//     }
//     {
//         Variant var;
//         var.set_stringvalue("example-001");
//         paramsMap->insert(std::make_pair("correlationid", var));
//     }

//     std::cout << "Starting async send..." << std::endl;
//     auto future = client.SendAsync(send_req);

//     // Do other work while waiting
//     std::cout << "Doing other work while send is processing..." << std::endl;
//     std::this_thread::sleep_for(std::chrono::milliseconds(100));

//     // Wait for result
//     auto status = future.get();
//     PrintStatus(status, "Async Send Resource");

//     // Give some time for callbacks to complete
//     std::this_thread::sleep_for(std::chrono::milliseconds(500));
// }

// Example: Streaming operations
void ExampleStreamingOperations(InterfacesClient &client)
{
    std::cout << "\n=== Streaming Operations Example ===" << std::endl;

    ActionRequest Act_req;
    auto act_map = Act_req.mutable_input()->mutable_keyvaluelist();
    {
        Variant var;
        var.set_stringvalue("start_streaming");
        act_map->insert(std::make_pair("action", var));
    }
    {
        Variant var;
        var.set_int32value(5);
        act_map->insert(std::make_pair("duration", var));
    }
    {
        Variant var;
        var.set_stringvalue("test_topic");
        act_map->insert(std::make_pair("topic", var));
    }
    auto params_map = Act_req.mutable_params()->mutable_keyvaluelist();
    {
        Variant var;
        var.set_stringvalue("stream_param");
        params_map->insert(std::make_pair("param1", var));
    }
    {
        Variant var;
        var.set_int32value(10);
        params_map->insert(std::make_pair("timeout", var));
    }
    {
        Variant var;
        var.set_stringvalue("stream-001");
        params_map->insert(std::make_pair("correlationid", var));
    }

    std::cout << "Starting action (will timeout after 5 seconds)..." << std::endl;

    int event_count = 0;
    ActionResponse act_resp;
    std::unique_ptr<::grpc::ClientReader<::ActionResponse>> reader;
    grpc::ClientContext context;
    context.set_deadline(GetDeadline(5000));

    auto status = client.Action(Act_req, reader, context);

    if (status)
    {
        std::cout << "[✓] Action completed normally" << std::endl;
    }
    else
    {
        std::cout << "[✗] Action ended with error: " << status.message() << std::endl;
    }

    std::cout << "Total events received: " << event_count << std::endl;
}

// Example: Error handling and connection management
void ExampleConnectionManagement()
{
    std::cout << "\n=== Connection Management Example ===" << std::endl;

    // Create client using convenience function (now using shared_ptr for async safety)
    std::shared_ptr<InterfacesClient> client;
    auto status = humanoid_robot::clientSDK::factory::CreateInterfacesClient("localhost", 50051, client);

    if (status)
    {
        std::cout << "[✓] Connected to server" << std::endl;

        // Check channel state
        auto state = client->GetChannelState(false);
        std::cout << "Channel state: " << state << std::endl;

        // Wait for channel to be ready
        if (client->WaitForChannelReady(5000))
        {
            std::cout << "[✓] Channel is ready" << std::endl;
        }
        else
        {
            std::cout << "[⚠] Channel not ready within timeout" << std::endl;
        }

        // Use the connected client
        ExampleSyncOperations(*client);
        // ExampleAsyncOperations(*client);
        ExampleStreamingOperations(*client);

        // Disconnect
        client->Disconnect();
        std::cout << "[✓] Disconnected from server" << std::endl;
    }
    else
    {
        std::cout << "[✗] Failed to connect: " << status.message() << std::endl;
        std::cout << "Note: Make sure the InterfaceService server is running on localhost:50051" << std::endl;
    }
}

using SubscriptionMessageCallback = std::function<void(const Notification &)>;

void NotificationHandler(const Notification &notification)
{
    std::cout << "[Notification Received] ";
    print_keyvaluelist(notification.notifymessage().keyvaluelist());
}

void ExampleSubscription()
{
    std::cout << "\n=== Subscription Example ===" << std::endl;
    // Create client using convenience function (now using shared_ptr for async safety)
    std::shared_ptr<InterfacesClient> client;
    auto status = humanoid_robot::clientSDK::factory::CreateInterfacesClient("localhost", 50051, client);
    if (status)
    {
        std::cout << "[✓] Connected to server" << std::endl;
    }
    else
    {
        std::cout << "[✗] Failed to connect: " << status.message() << std::endl;
        return;
    }
    // Create and start callback server
    Status server_status;
    auto callback_server = humanoid_robot::clientSDK::robot::CreateCallbackServer(
        "127.0.0.1",
        server_status,
        50052, // 固定端口
        [](const Notification &notification)
        {
            NotificationHandler(notification);
        });

    if (server_status)
    {
        std::cout << "[✓] Callback server started successfully" << std::endl;
    }
    else
    {
        std::cout << "[✗] Failed to start callback server: " << server_status.message() << std::endl;
        return;
    }
    // Subscribe to notifications
    SubscribeRequest sub_req;
    SubscribeResponse sub_resp;
    auto input_map = sub_req.mutable_input()->mutable_keyvaluelist();
    {
        Variant var;
        var.set_stringvalue("127.0.0.1:50052");
        input_map->insert(std::make_pair("client_endpoint", var));
    }
    status = client->Subscribe(sub_req, sub_resp, 1000);
    if (status)
    {
        std::cout << "[✓] Subscribed to notifications" << std::endl;
    }
    else
    {
        std::cout << "[✗] Failed to subscribe: " << status.message() << std::endl;
    }
    std::this_thread::sleep_for(std::chrono::seconds(60000)); // 等待5秒后发送模拟通知
}

int main()
{
    WLOG_DEBUG("Start of main function", 1);

    // std::cout << "InterfacesClient Example" << std::endl;
    // std::cout << "========================" << std::endl;

    // try
    // {
    //     ExampleSubscription();
    //     // ExampleConnectionManagement();
    // }
    // catch (const std::exception &e)
    // {
    //     std::cerr << "Exception: " << e.what() << std::endl;
    //     return 1;
    // }
    // std::this_thread::sleep_for(std::chrono::seconds(20000)); // Give time for async operations to complete
    // std::cout << "\n[✓] Example completed" << std::endl;
    return 0;
}
