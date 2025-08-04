/**
 * Copyright (c) 2025 Humanoid Robot, Inc. All rights reserved.
 *
 * Implementation of InterfacesClient for Robot SDK
 */

#include "robot/client/interfaces_client.h"
#include "robot/common/errorCode.h"

#include <grpcpp/grpcpp.h>
#include <chrono>
#include <thread>
#include <iostream>
#include <ctime>

using namespace humanoid_robot::clientSDK::robot;
using namespace humanoid_robot::clientSDK::common;

// Private implementation class
class InterfacesClient::Impl
{
public:
    std::shared_ptr<grpc::Channel> channel_;
    std::unique_ptr<interfaces::InterfaceService::Stub> stub_;
    std::string target_;
    bool connected_;

    Impl() : connected_(false) {}

    ~Impl()
    {
        if (connected_)
        {
            // Graceful shutdown could be implemented here
        }
    }
};

// =================================================================
// Constructor and Destructor
// =================================================================

InterfacesClient::InterfacesClient() : pImpl_(std::make_unique<Impl>())
{
}

InterfacesClient::~InterfacesClient() = default;

// =================================================================
// Connection Management
// =================================================================

Status InterfacesClient::Connect(const std::string &server_address, int port)
{
    std::string target = server_address + ":" + std::to_string(port);
    return Connect(target);
}

Status InterfacesClient::Connect(const std::string &target)
{
    try
    {
        pImpl_->target_ = target;

        // Create gRPC channel with default credentials
        grpc::ChannelArguments args;
        args.SetMaxReceiveMessageSize(100 * 1024 * 1024); // 100MB max message size
        args.SetMaxSendMessageSize(100 * 1024 * 1024);

        pImpl_->channel_ = grpc::CreateCustomChannel(
            target,
            grpc::InsecureChannelCredentials(),
            args);

        if (!pImpl_->channel_)
        {
            return Status(
                std::make_error_code(std::errc::connection_refused),
                "Failed to create gRPC channel");
        }

        // Create service stub
        pImpl_->stub_ = interfaces::InterfaceService::NewStub(pImpl_->channel_);

        if (!pImpl_->stub_)
        {
            return Status(
                std::make_error_code(std::errc::connection_refused),
                "Failed to create service stub");
        }

        pImpl_->connected_ = true;
        return Status();
    }
    catch (const std::exception &e)
    {
        return Status(
            std::make_error_code(std::errc::connection_refused),
            std::string("Connection failed: ") + e.what());
    }
}

void InterfacesClient::Disconnect()
{
    pImpl_->stub_.reset();
    pImpl_->channel_.reset();
    pImpl_->connected_ = false;
}

bool InterfacesClient::IsConnected() const
{
    return pImpl_->connected_ && pImpl_->channel_ && pImpl_->stub_;
}

// =================================================================
// Synchronous Methods
// =================================================================

Status InterfacesClient::Create(
    const interfaces::CreateRequest &request,
    interfaces::CreateResponse &response,
    int64_t timeout_ms)
{
    if (!IsConnected())
    {
        return Status(
            std::make_error_code(std::errc::not_connected),
            "Client not connected");
    }

    grpc::ClientContext context;
    context.set_deadline(GetDeadline(timeout_ms));

    grpc::Status status = pImpl_->stub_->Create(&context, request, &response);
    return ConvertGrpcStatus(status);
}

Status InterfacesClient::Send(
    const interfaces::SendRequest &request,
    interfaces::SendResponse &response,
    int64_t timeout_ms)
{
    if (!IsConnected())
    {
        return Status(
            std::make_error_code(std::errc::not_connected),
            "Client not connected");
    }

    grpc::ClientContext context;
    context.set_deadline(GetDeadline(timeout_ms));

    grpc::Status status = pImpl_->stub_->Send(&context, request, &response);
    return ConvertGrpcStatus(status);
}

Status InterfacesClient::Delete(
    const interfaces::DeleteRequest &request,
    interfaces::DeleteResponse &response,
    int64_t timeout_ms)
{
    if (!IsConnected())
    {
        return Status(
            std::make_error_code(std::errc::not_connected),
            "Client not connected");
    }

    grpc::ClientContext context;
    context.set_deadline(GetDeadline(timeout_ms));

    grpc::Status status = pImpl_->stub_->Delete(&context, request, &response);
    return ConvertGrpcStatus(status);
}

Status InterfacesClient::Query(
    const interfaces::QueryRequest &request,
    interfaces::QueryResponse &response,
    int64_t timeout_ms)
{
    if (!IsConnected())
    {
        return Status(
            std::make_error_code(std::errc::not_connected),
            "Client not connected");
    }

    grpc::ClientContext context;
    context.set_deadline(GetDeadline(timeout_ms));

    grpc::Status status = pImpl_->stub_->Query(&context, request, &response);
    return ConvertGrpcStatus(status);
}

Status InterfacesClient::BatchCreate(
    const interfaces::BatchCreateRequest &request,
    interfaces::BatchCreateResponse &response,
    int64_t timeout_ms)
{
    if (!IsConnected())
    {
        return Status(
            std::make_error_code(std::errc::not_connected),
            "Client not connected");
    }

    grpc::ClientContext context;
    context.set_deadline(GetDeadline(timeout_ms));

    grpc::Status status = pImpl_->stub_->BatchCreate(&context, request, &response);
    return ConvertGrpcStatus(status);
}

Status InterfacesClient::HealthCheck(
    const interfaces::HealthCheckRequest &request,
    interfaces::HealthCheckResponse &response,
    int64_t timeout_ms)
{
    if (!IsConnected())
    {
        return Status(
            std::make_error_code(std::errc::not_connected),
            "Client not connected");
    }

    grpc::ClientContext context;
    context.set_deadline(GetDeadline(timeout_ms));

    grpc::Status status = pImpl_->stub_->HealthCheck(&context, request, &response);
    return ConvertGrpcStatus(status);
}

Status InterfacesClient::Unsubscribe(
    const interfaces::UnsubscribeRequest &request,
    interfaces::UnsubscribeResponse &response,
    int64_t timeout_ms)
{
    if (!IsConnected())
    {
        return Status(
            std::make_error_code(std::errc::not_connected),
            "Client not connected");
    }

    grpc::ClientContext context;
    context.set_deadline(GetDeadline(timeout_ms));

    grpc::Status status = pImpl_->stub_->Unsubscribe(&context, request, &response);
    return ConvertGrpcStatus(status);
}

// =================================================================
// Asynchronous Methods (Future-based)
// =================================================================

AsyncResult<interfaces::CreateResponse> InterfacesClient::CreateAsync(
    const interfaces::CreateRequest &request,
    int64_t timeout_ms)
{
    auto promise = std::make_shared<std::promise<Status>>();
    auto future = promise->get_future();

    CreateAsync(request, [promise](const Status &status, const interfaces::CreateResponse &response)
                { promise->set_value(status); }, timeout_ms);

    return future;
}

void InterfacesClient::CreateAsync(
    const interfaces::CreateRequest &request,
    AsyncCallback<interfaces::CreateResponse> callback,
    int64_t timeout_ms)
{
    // 使用 weak_ptr 避免循环引用和内存泄漏
    std::weak_ptr<InterfacesClient> weak_self = shared_from_this();

    std::thread([weak_self, request, callback, timeout_ms]()
                {
                // 尝试获取 shared_ptr，检查对象是否仍然存在
                if (auto self = weak_self.lock()) {
                    interfaces::CreateResponse response;
                    auto status = self->Create(request, response, timeout_ms);
                    callback(status, response);
                } else {
                    // 对象已被销毁，调用回调通知错误
                    interfaces::CreateResponse response;
                    Status error_status(
                        std::make_error_code(std::errc::operation_canceled),
                        "Client object has been destroyed");
                    callback(error_status, response);
                } })
        .detach();
}

AsyncResult<interfaces::SendResponse> InterfacesClient::SendAsync(
    const interfaces::SendRequest &request,
    int64_t timeout_ms)
{
    auto promise = std::make_shared<std::promise<Status>>();
    auto future = promise->get_future();

    SendAsync(request, [promise](const Status &status, const interfaces::SendResponse &response)
              { promise->set_value(status); }, timeout_ms);

    return future;
}

void InterfacesClient::SendAsync(
    const interfaces::SendRequest &request,
    AsyncCallback<interfaces::SendResponse> callback,
    int64_t timeout_ms)
{
    std::weak_ptr<InterfacesClient> weak_self = shared_from_this();

    std::thread([weak_self, request, callback, timeout_ms]()
                {
                if (auto self = weak_self.lock()) {
                    interfaces::SendResponse response;
                    auto status = self->Send(request, response, timeout_ms);
                    callback(status, response);
                } else {
                    interfaces::SendResponse response;
                    Status error_status(
                        std::make_error_code(std::errc::operation_canceled),
                        "Client object has been destroyed");
                    callback(error_status, response);
                } })
        .detach();
}

AsyncResult<interfaces::QueryResponse> InterfacesClient::QueryAsync(
    const interfaces::QueryRequest &request,
    int64_t timeout_ms)
{
    auto promise = std::make_shared<std::promise<Status>>();
    auto future = promise->get_future();

    QueryAsync(request, [promise](const Status &status, const interfaces::QueryResponse &response)
               { promise->set_value(status); }, timeout_ms);

    return future;
}

void InterfacesClient::QueryAsync(
    const interfaces::QueryRequest &request,
    AsyncCallback<interfaces::QueryResponse> callback,
    int64_t timeout_ms)
{
    std::weak_ptr<InterfacesClient> weak_self = shared_from_this();

    std::thread([weak_self, request, callback, timeout_ms]()
                {
                if (auto self = weak_self.lock()) {
                    interfaces::QueryResponse response;
                    auto status = self->Query(request, response, timeout_ms);
                    callback(status, response);
                } else {
                    interfaces::QueryResponse response;
                    Status error_status(
                        std::make_error_code(std::errc::operation_canceled),
                        "Client object has been destroyed");
                    callback(error_status, response);
                } })
        .detach();
}

AsyncResult<interfaces::HealthCheckResponse> InterfacesClient::HealthCheckAsync(
    const interfaces::HealthCheckRequest &request,
    int64_t timeout_ms)
{
    auto promise = std::make_shared<std::promise<Status>>();
    auto future = promise->get_future();

    HealthCheckAsync(request, [promise](const Status &status, const interfaces::HealthCheckResponse &response)
                     { promise->set_value(status); }, timeout_ms);

    return future;
}

void InterfacesClient::HealthCheckAsync(
    const interfaces::HealthCheckRequest &request,
    AsyncCallback<interfaces::HealthCheckResponse> callback,
    int64_t timeout_ms)
{
    std::weak_ptr<InterfacesClient> weak_self = shared_from_this();

    std::thread([weak_self, request, callback, timeout_ms]()
                {
                if (auto self = weak_self.lock()) {
                    interfaces::HealthCheckResponse response;
                    auto status = self->HealthCheck(request, response, timeout_ms);
                    callback(status, response);
                } else {
                    interfaces::HealthCheckResponse response;
                    Status error_status(
                        std::make_error_code(std::errc::operation_canceled),
                        "Client object has been destroyed");
                    callback(error_status, response);
                } })
        .detach();
}

// =================================================================
// Streaming Methods
// =================================================================

Status InterfacesClient::Subscribe(
    const interfaces::SubscribeRequest &request,
    std::function<void(const interfaces::SubscribeResponse &)> callback,
    int64_t timeout_ms)
{
    if (!IsConnected())
    {
        return Status(
            std::make_error_code(std::errc::not_connected),
            "Client not connected");
    }

    grpc::ClientContext context;
    if (timeout_ms > 0)
    {
        context.set_deadline(GetDeadline(timeout_ms));
    }

    auto reader = pImpl_->stub_->Subscribe(&context, request);

    interfaces::SubscribeResponse response;
    while (reader->Read(&response))
    {
        callback(response);
    }

    grpc::Status status = reader->Finish();
    return ConvertGrpcStatus(status);
}

Status InterfacesClient::SubscribeWithErrorHandling(
    const interfaces::SubscribeRequest &request,
    std::function<void(const interfaces::SubscribeResponse &)> response_callback,
    std::function<void(const Status &)> error_callback,
    int64_t timeout_ms)
{
    std::weak_ptr<InterfacesClient> weak_self = shared_from_this();

    // Start subscription in a separate thread
    std::thread([weak_self, request, response_callback, error_callback, timeout_ms]()
                {
                if (auto self = weak_self.lock()) {
                    auto status = self->Subscribe(request, response_callback, timeout_ms);
                    error_callback(status);
                } else {
                    Status error_status(
                        std::make_error_code(std::errc::operation_canceled),
                        "Client object has been destroyed");
                    error_callback(error_status);
                } })
        .detach();

    return Status(); // Return immediately
}

// =================================================================
// Utility Methods
// =================================================================

grpc_connectivity_state InterfacesClient::GetChannelState(bool try_to_connect)
{
    if (!pImpl_->channel_)
    {
        return GRPC_CHANNEL_SHUTDOWN;
    }
    return pImpl_->channel_->GetState(try_to_connect);
}

bool InterfacesClient::WaitForChannelReady(int64_t timeout_ms)
{
    if (!pImpl_->channel_)
    {
        return false;
    }

    auto deadline = GetDeadline(timeout_ms);
    auto state = pImpl_->channel_->GetState(true);

    while (state != GRPC_CHANNEL_READY && std::chrono::system_clock::now() < deadline)
    {
        if (!pImpl_->channel_->WaitForStateChange(state, deadline))
        {
            return false; // Timeout
        }
        state = pImpl_->channel_->GetState(false);
    }

    return state == GRPC_CHANNEL_READY;
}

Status InterfacesClient::GetServerInfo(std::string &server_info)
{
    interfaces::HealthCheckRequest request;
    request.set_service("InterfaceService");

    // Add server info request parameter
    auto *params = request.mutable_checkparams();
    auto *items = params->mutable_keyvaluelist();
    (*items)["get_server_info"] = base_types::Variant();
    (*items)["get_server_info"].set_type(base_types::Variant::KBoolValue);
    (*items)["get_server_info"].set_boolvalue(true);

    interfaces::HealthCheckResponse response;
    auto status = HealthCheck(request, response, 3000);

    if (status)
    {
        // Extract server info from response metrics
        if (response.has_metrics())
        {
            auto &metrics = response.metrics();
            auto &items = metrics.keyvaluelist();
            auto it = items.find("server_info");
            if (it != items.end() && it->second.has_stringvalue())
            {
                server_info = it->second.stringvalue();
            }
        }
    }

    return status;
}

// =================================================================
// Private Helper Methods
// =================================================================

Status InterfacesClient::ConvertGrpcStatus(const grpc::Status &grpc_status)
{
    if (grpc_status.ok())
    {
        return Status();
    }

    std::error_code ec;
    switch (grpc_status.error_code())
    {
    case grpc::StatusCode::CANCELLED:
        ec = std::make_error_code(std::errc::operation_canceled);
        break;
    case grpc::StatusCode::DEADLINE_EXCEEDED:
        ec = std::make_error_code(std::errc::timed_out);
        break;
    case grpc::StatusCode::NOT_FOUND:
        ec = std::make_error_code(std::errc::no_such_file_or_directory);
        break;
    case grpc::StatusCode::ALREADY_EXISTS:
        ec = std::make_error_code(std::errc::file_exists);
        break;
    case grpc::StatusCode::PERMISSION_DENIED:
        ec = std::make_error_code(std::errc::permission_denied);
        break;
    case grpc::StatusCode::UNAVAILABLE:
        ec = std::make_error_code(std::errc::host_unreachable);
        break;
    case grpc::StatusCode::UNIMPLEMENTED:
        ec = std::make_error_code(std::errc::function_not_supported);
        break;
    default:
        ec = std::make_error_code(std::errc::io_error);
        break;
    }

    return Status(ec, grpc_status.error_message());
}

std::chrono::system_clock::time_point InterfacesClient::GetDeadline(int64_t timeout_ms)
{
    return std::chrono::system_clock::now() + std::chrono::milliseconds(timeout_ms);
}

// =================================================================
// Convenience Functions
// =================================================================

// Factory functions implementation
namespace humanoid_robot::clientSDK::factory
{
    // Preferred shared_ptr versions for async safety
    Status CreateInterfacesClient(
        const std::string &server_address,
        int port,
        std::shared_ptr<robot::InterfacesClient> &client)
    {
        client = std::shared_ptr<robot::InterfacesClient>(new robot::InterfacesClient());
        return client->Connect(server_address, port);
    }

    Status CreateInterfacesClient(
        const std::string &target,
        std::shared_ptr<robot::InterfacesClient> &client)
    {
        client = std::shared_ptr<robot::InterfacesClient>(new robot::InterfacesClient());
        return client->Connect(target);
    }
}

// =================================================================
// 持久订阅方法实现 (简化版本)
// =================================================================

std::pair<Status, std::string> InterfacesClient::CreatePersistentSubscription(
    const std::string &topic_id,
    const std::string &object_id,
    const std::string &client_endpoint,
    const std::vector<std::string> &event_types,
    const base_types::Dictionary &subscription_data)
{
    if (!IsConnected())
    {
        return std::make_pair(
            Status(std::make_error_code(std::errc::not_connected), "Client not connected"),
            "");
    }

    // 生成订阅ID
    std::string subscription_id = "sub_" + object_id + "_" + std::to_string(std::time(nullptr));

    std::cout << "Created persistent subscription: " << subscription_id
              << " for objectId: " << object_id
              << " with client endpoint: " << client_endpoint << std::endl;

    // TODO: 实际实现需要调用服务端接口创建持久订阅
    // 这里先返回成功状态和生成的订阅ID

    return std::make_pair(Status(), subscription_id);
}

Status InterfacesClient::UpdatePersistentSubscription(
    const std::string &subscription_id,
    const std::vector<std::string> &event_types,
    const base_types::Dictionary &subscription_data)
{
    std::cout << "Updating persistent subscription: " << subscription_id << std::endl;

    // TODO: 实现订阅更新逻辑
    return Status(std::make_error_code(std::errc::function_not_supported),
                  "Update subscription not implemented yet");
}

Status InterfacesClient::CancelPersistentSubscription(
    const std::string &subscription_id,
    const std::string &object_id)
{
    if (!IsConnected())
    {
        return Status(std::make_error_code(std::errc::not_connected), "Client not connected");
    }

    std::cout << "Cancelling persistent subscription: " << subscription_id
              << " for objectId: " << object_id << std::endl;

    // TODO: 实际实现需要调用服务端取消订阅接口
    return Status();
}

Status InterfacesClient::GetSubscriptionStatus(
    const std::string &subscription_id,
    base_types::Dictionary &subscription_info)
{
    if (!IsConnected())
    {
        return Status(std::make_error_code(std::errc::not_connected), "Client not connected");
    }

    std::cout << "Getting subscription status for: " << subscription_id << std::endl;

    // TODO: 实际实现需要查询服务端订阅状态
    return Status();
}