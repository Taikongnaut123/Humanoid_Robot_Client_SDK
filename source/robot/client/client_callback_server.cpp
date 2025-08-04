/**
 * Copyright (c) 2025 Humanoid Robot, Inc. All rights reserved.
 *
 * Implementation of ClientCallbackServer
 */

#include "robot/client/client_callback_server.h"
#include "robot/common/errorCode.h"

#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>

#include <iostream>
#include <sstream>

using namespace humanoid_robot::clientSDK::robot;
using namespace humanoid_robot::clientSDK::common;

// =============================================================================
// ClientCallbackServiceImpl - gRPC服务实现
// =============================================================================

class ClientCallbackServiceImpl final : public interfaces::ClientCallbackService::Service
{
public:
    ClientCallbackServiceImpl(
        SubscriptionMessageCallback message_callback,
        SubscriptionStatusCallback status_callback,
        SubscriptionErrorCallback error_callback)
        : message_callback_(message_callback), status_callback_(status_callback), error_callback_(error_callback)
    {
    }

    grpc::Status OnSubscriptionMessage(
        grpc::ServerContext *context,
        const interfaces::SubscriptionNotification *request,
        interfaces::NotificationAck *response) override
    {
        std::cout << "Received subscription message for objectId: " << request->objectid()
                  << ", messageId: " << request->messageid() << std::endl;

        if (message_callback_)
        {
            try
            {
                message_callback_(*request);
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error in message callback: " << e.what() << std::endl;
                response->set_received(false);
                response->set_message("Callback execution failed: " + std::string(e.what()));
                return grpc::Status::OK;
            }
        }

        // 设置确认响应
        response->set_received(true);
        response->set_ackid(request->messageid());
        response->set_message("Message received successfully");
        response->set_timestamp(std::chrono::duration_cast<std::chrono::milliseconds>(
                                    std::chrono::system_clock::now().time_since_epoch())
                                    .count());

        return grpc::Status::OK;
    }

    grpc::Status OnSubscriptionStatusChange(
        grpc::ServerContext *context,
        const interfaces::SubscriptionStatusChange *request,
        interfaces::NotificationAck *response) override
    {
        std::cout << "Received status change for objectId: " << request->objectid()
                  << ", status: " << request->oldstatus() << " -> " << request->newstatus() << std::endl;

        if (status_callback_)
        {
            try
            {
                status_callback_(*request);
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error in status callback: " << e.what() << std::endl;
                response->set_received(false);
                response->set_message("Callback execution failed: " + std::string(e.what()));
                return grpc::Status::OK;
            }
        }

        response->set_received(true);
        response->set_ackid(request->subscriptionid());
        response->set_message("Status change received successfully");
        response->set_timestamp(std::chrono::duration_cast<std::chrono::milliseconds>(
                                    std::chrono::system_clock::now().time_since_epoch())
                                    .count());

        return grpc::Status::OK;
    }

    grpc::Status OnSubscriptionError(
        grpc::ServerContext *context,
        const interfaces::SubscriptionError *request,
        interfaces::NotificationAck *response) override
    {
        std::cout << "Received subscription error for objectId: " << request->objectid()
                  << ", error: " << request->error().message() << std::endl;

        if (error_callback_)
        {
            try
            {
                error_callback_(*request);
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error in error callback: " << e.what() << std::endl;
                response->set_received(false);
                response->set_message("Callback execution failed: " + std::string(e.what()));
                return grpc::Status::OK;
            }
        }

        response->set_received(true);
        response->set_ackid(request->subscriptionid());
        response->set_message("Error notification received successfully");
        response->set_timestamp(std::chrono::duration_cast<std::chrono::milliseconds>(
                                    std::chrono::system_clock::now().time_since_epoch())
                                    .count());

        return grpc::Status::OK;
    }

private:
    SubscriptionMessageCallback message_callback_;
    SubscriptionStatusCallback status_callback_;
    SubscriptionErrorCallback error_callback_;
};

// =============================================================================
// ClientCallbackServer::Impl - 私有实现
// =============================================================================

class ClientCallbackServer::Impl
{
public:
    std::unique_ptr<grpc::Server> server_;
    std::unique_ptr<ClientCallbackServiceImpl> service_impl_;
    std::string listen_address_;
    int listen_port_;
    std::atomic<bool> running_;
    std::thread server_thread_;

    // 回调函数
    SubscriptionMessageCallback message_callback_;
    SubscriptionStatusCallback status_callback_;
    SubscriptionErrorCallback error_callback_;

    Impl() : listen_port_(0), running_(false) {}

    ~Impl()
    {
        Stop();
    }

    void Stop()
    {
        running_ = false;
        if (server_)
        {
            server_->Shutdown();
            if (server_thread_.joinable())
            {
                server_thread_.join();
            }
        }
    }
};

// =============================================================================
// ClientCallbackServer 实现
// =============================================================================

ClientCallbackServer::ClientCallbackServer() : pImpl_(std::make_unique<Impl>())
{
}

ClientCallbackServer::~ClientCallbackServer() = default;

Status ClientCallbackServer::Start(const std::string &listen_address, int port)
{
    if (pImpl_->running_)
    {
        return Status(
            std::make_error_code(std::errc::operation_not_permitted),
            "Server is already running");
    }

    try
    {
        pImpl_->listen_address_ = listen_address;
        pImpl_->listen_port_ = port;

        // 构建监听地址
        std::string server_address = listen_address + ":" + std::to_string(port);

        // 创建服务实现
        pImpl_->service_impl_ = std::make_unique<ClientCallbackServiceImpl>(
            pImpl_->message_callback_,
            pImpl_->status_callback_,
            pImpl_->error_callback_);

        // 构建服务器
        grpc::ServerBuilder builder;
        builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
        builder.RegisterService(pImpl_->service_impl_.get());

        // 启用健康检查和反射（可选）
        grpc::EnableDefaultHealthCheckService(true);
        grpc::reflection::InitProtoReflectionServerBuilderPlugin();

        // 构建并启动服务器
        pImpl_->server_ = builder.BuildAndStart();

        if (!pImpl_->server_)
        {
            return Status(
                std::make_error_code(std::errc::address_not_available),
                "Failed to start gRPC callback server");
        }

        pImpl_->running_ = true;

        // 在单独线程中运行服务器
        pImpl_->server_thread_ = std::thread([this]()
                                             {
            std::cout << "Client callback server listening on " 
                      << pImpl_->listen_address_ << ":" << pImpl_->listen_port_ << std::endl;
            pImpl_->server_->Wait(); });

        return Status();
    }
    catch (const std::exception &e)
    {
        return Status(
            std::make_error_code(std::errc::operation_not_supported),
            std::string("Failed to start callback server: ") + e.what());
    }
}

Status ClientCallbackServer::StartWithAutoPort(const std::string &listen_address, int &assigned_port)
{
    // 使用端口0让系统自动分配端口
    auto status = Start(listen_address, 0);
    if (status)
    {
        // TODO: 这里需要实际获取分配的端口号
        // 目前简化实现，使用默认端口范围
        assigned_port = 0; // 需要从gRPC服务器获取实际端口
        pImpl_->listen_port_ = assigned_port;
    }
    return status;
}

void ClientCallbackServer::Stop()
{
    pImpl_->Stop();
}

bool ClientCallbackServer::IsRunning() const
{
    return pImpl_->running_;
}

std::string ClientCallbackServer::GetListenAddress() const
{
    return pImpl_->listen_address_;
}

int ClientCallbackServer::GetListenPort() const
{
    return pImpl_->listen_port_;
}

void ClientCallbackServer::SetSubscriptionMessageCallback(SubscriptionMessageCallback callback)
{
    pImpl_->message_callback_ = callback;
    if (pImpl_->service_impl_)
    {
        // 如果服务已经创建，需要重新创建服务实现
        // 在实际使用中，建议在启动前设置所有回调
    }
}

void ClientCallbackServer::SetSubscriptionStatusCallback(SubscriptionStatusCallback callback)
{
    pImpl_->status_callback_ = callback;
}

void ClientCallbackServer::SetSubscriptionErrorCallback(SubscriptionErrorCallback callback)
{
    pImpl_->error_callback_ = callback;
}

void ClientCallbackServer::SetAllCallbacks(
    SubscriptionMessageCallback message_callback,
    SubscriptionStatusCallback status_callback,
    SubscriptionErrorCallback error_callback)
{
    pImpl_->message_callback_ = message_callback;
    pImpl_->status_callback_ = status_callback;
    pImpl_->error_callback_ = error_callback;
}

std::string ClientCallbackServer::GetClientEndpoint() const
{
    if (pImpl_->listen_address_.empty() || pImpl_->listen_port_ == 0)
    {
        return "";
    }
    return pImpl_->listen_address_ + ":" + std::to_string(pImpl_->listen_port_);
}

// =============================================================================
// 便捷工厂函数
// =============================================================================

std::pair<std::unique_ptr<ClientCallbackServer>, Status>
humanoid_robot::clientSDK::robot::CreateCallbackServer(
    const std::string &listen_address,
    int port,
    SubscriptionMessageCallback message_callback,
    SubscriptionStatusCallback status_callback,
    SubscriptionErrorCallback error_callback)
{
    auto server = std::make_unique<ClientCallbackServer>();

    // 设置回调
    if (message_callback || status_callback || error_callback)
    {
        server->SetAllCallbacks(message_callback, status_callback, error_callback);
    }

    // 启动服务器
    Status status;
    if (port == 0)
    {
        int assigned_port;
        status = server->StartWithAutoPort(listen_address, assigned_port);
    }
    else
    {
        status = server->Start(listen_address, port);
    }

    return std::make_pair(std::move(server), status);
}
