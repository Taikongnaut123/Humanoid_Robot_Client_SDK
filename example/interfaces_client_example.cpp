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

using namespace humanoid_robot::clientSDK::robot;
using namespace humanoid_robot::clientSDK::common;

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

    // 1. Health Check
    interfaces::HealthCheckRequest health_req;
    health_req.set_service("InterfaceService");

    interfaces::HealthCheckResponse health_resp;
    auto status = client.HealthCheck(health_req, health_resp);
    PrintStatus(status, "Health Check");

    if (status)
    {
        std::cout << "    Server status: " << static_cast<int>(health_resp.status()) << std::endl;
        std::cout << "    Message: " << health_resp.message() << std::endl;
    }

    // 2. Create Resource
    interfaces::CreateRequest create_req;

    // Set request data
    auto *req_data = create_req.mutable_requestdata();
    auto *req_items = req_data->mutable_keyvaluelist();

    // Add name field
    base_types::Variant name_variant;
    name_variant.set_type(base_types::Variant::KStringValue);
    name_variant.set_stringvalue("example_resource");
    (*req_items)["name"] = name_variant;

    // Add value field
    base_types::Variant value_variant;
    value_variant.set_type(base_types::Variant::KInt32Value);
    value_variant.set_int32value(42);
    (*req_items)["value"] = value_variant;

    // Set parameters
    auto *params = create_req.mutable_params();
    params->set_timeout(30);
    params->set_correlationid("example-001");

    interfaces::CreateResponse create_resp;
    status = client.Create(create_req, create_resp);
    PrintStatus(status, "Create Resource");

    if (status)
    {
        std::cout << "    Resource ID: " << create_resp.resourceid() << std::endl;
        std::cout << "    Status: " << static_cast<int>(create_resp.status()) << std::endl;
        std::cout << "    Message: " << create_resp.message() << std::endl;
    }

    // 3. Query Resources
    interfaces::QueryRequest query_req;
    query_req.set_queryid("find_resources");
    query_req.set_limit(10);
    query_req.set_offset(0);

    interfaces::QueryResponse query_resp;
    status = client.Query(query_req, query_resp);
    PrintStatus(status, "Query Resources");

    if (status)
    {
        std::cout << "    Found " << query_resp.totalcount() << " resources" << std::endl;
        std::cout << "    Returned " << query_resp.results_size() << " results" << std::endl;
    }

    // 4. Send Message
    interfaces::SendRequest send_req;
    send_req.set_targetid("service_endpoint");

    auto *msg_data = send_req.mutable_messagedata();
    auto *msg_items = msg_data->mutable_keyvaluelist();

    // Add message field
    base_types::Variant msg_variant;
    msg_variant.set_type(base_types::Variant::KStringValue);
    msg_variant.set_stringvalue("Hello from InterfacesClient!");
    (*msg_items)["message"] = msg_variant;

    interfaces::SendResponse send_resp;
    status = client.Send(send_req, send_resp);
    PrintStatus(status, "Send Message");

    if (status)
    {
        std::cout << "    Response: " << send_resp.message() << std::endl;
    }
}

// Example: Asynchronous operations
void ExampleAsyncOperations(InterfacesClient &client)
{
    std::cout << "\n=== Asynchronous Operations Example ===" << std::endl;

    // Async Health Check with callback
    interfaces::HealthCheckRequest health_req;
    health_req.set_service("InterfaceService");

    std::cout << "Starting async health check..." << std::endl;
    client.HealthCheckAsync(health_req, [](const Status &status, const interfaces::HealthCheckResponse &response)
                            {
            if (status) {
                std::cout << "[✓] Async health check completed: " << response.message() << std::endl;
            } else {
                std::cout << "[✗] Async health check failed: " << status.message() << std::endl;
            } }, 3000);

    // Async Create with future
    interfaces::CreateRequest create_req;

    auto *req_data = create_req.mutable_requestdata();
    auto *req_items = req_data->mutable_keyvaluelist();

    base_types::Variant async_variant;
    async_variant.set_type(base_types::Variant::KBoolValue);
    async_variant.set_boolvalue(true);
    (*req_items)["async"] = async_variant;

    std::cout << "Starting async create..." << std::endl;
    auto future = client.CreateAsync(create_req);

    // Do other work while waiting
    std::cout << "Doing other work while create is processing..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Wait for result
    auto status = future.get();
    PrintStatus(status, "Async Create Resource");

    // Give some time for callbacks to complete
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

// Example: Streaming operations
void ExampleStreamingOperations(InterfacesClient &client)
{
    std::cout << "\n=== Streaming Operations Example ===" << std::endl;

    interfaces::SubscribeRequest sub_req;
    sub_req.set_topicid("test_topic");

    // Set subscription parameters
    auto *sub_params = sub_req.mutable_params();
    sub_params->set_timeout(5); // 5 second subscription
    sub_params->set_correlationid("stream-001");

    std::cout << "Starting subscription (will timeout after 5 seconds)..." << std::endl;

    int event_count = 0;
    auto status = client.Subscribe(sub_req, [&event_count](const interfaces::SubscribeResponse &response)
                                   {
            ++event_count;
            std::cout << "[Event " << event_count << "] Received subscription response: " 
                      << response.message() << std::endl; }, 5000); // 5 second timeout

    if (status)
    {
        std::cout << "[✓] Subscription completed normally" << std::endl;
    }
    else
    {
        std::cout << "[✗] Subscription ended with error: " << status.message() << std::endl;
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

        // Get server info
        std::string server_info;
        status = client->GetServerInfo(server_info);
        if (status && !server_info.empty())
        {
            std::cout << "Server info: " << server_info << std::endl;
        }

        // Use the connected client
        ExampleSyncOperations(*client);
        ExampleAsyncOperations(*client);
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

int main()
{
    std::cout << "InterfacesClient Example" << std::endl;
    std::cout << "========================" << std::endl;

    try
    {
        ExampleConnectionManagement();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "\n[✓] Example completed" << std::endl;
    return 0;
}
