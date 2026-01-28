/**
 * Copyright (c) 2025 Humanoid Robot, Inc. All rights reserved.
 *
 * Implementation of ClientCallbackServer
 */

#include "robot/client/client_callback_server.h"
#include "robot/common/errorCode.h"

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include <iostream>
#include <sstream>

using namespace humanoid_robot::clientSDK::robot;
using namespace humanoid_robot::clientSDK::common;
using namespace humanoid_robot::PB::interfaces;

// =============================================================================
// ClientCallbackServiceImpl - gRPC服务实现
// =============================================================================

class ClientCallbackServiceImpl final : public ClientCallbackService::Service {
public:
  ClientCallbackServiceImpl(SubscriptionMessageCallback message_callback)
      : message_callback_(message_callback) {}

  grpc::Status OnSubscriptionMessage(grpc::ServerContext *context,
                                     const Notification *request,
                                     NotificationAck *response) override {
    std::cout << "Received subscription message from client: " << std::endl;
    if (message_callback_) {
      try {
        message_callback_(*request);
      } catch (const std::exception &e) {
        std::cerr << "Error in message callback: " << e.what() << std::endl;
        response->set_ret(
            -0600060001); // 回调执行失败,客户端不关心服务器处理结果，可以不设置ret
        return grpc::Status::OK;
      }
    }

    // 设置确认响应
    response->set_ret(0);

    return grpc::Status::OK;
  }

private:
  SubscriptionMessageCallback message_callback_;
};

// =============================================================================
// ClientCallbackServer::Impl - 私有实现
// =============================================================================

class ClientCallbackServer::Impl {
public:
  std::unique_ptr<grpc::Server> server_;
  std::unique_ptr<ClientCallbackServiceImpl> service_impl_;
  std::string listen_address_;
  int listen_port_;
  std::atomic<bool> running_;
  std::thread server_thread_;

  // 回调函数
  SubscriptionMessageCallback message_callback_;

  Impl() : listen_port_(0), running_(false) {}

  ~Impl() { Stop(); }

  void Stop() {
    running_ = false;
    if (server_) {
      server_->Shutdown();
      if (server_thread_.joinable()) {
        server_thread_.join();
      }
    }
  }
};

// =============================================================================
// ClientCallbackServer 实现
// =============================================================================

ClientCallbackServer::ClientCallbackServer()
    : pImpl_(std::make_unique<Impl>()) {}

ClientCallbackServer::~ClientCallbackServer() = default;

Status ClientCallbackServer::Start(const std::string &listen_address,
                                   int port) {
  if (pImpl_->running_) {
    return Status(std::make_error_code(std::errc::operation_not_permitted),
                  "Server is already running");
  }

  try {
    pImpl_->listen_address_ = listen_address;
    pImpl_->listen_port_ = port;

    // 构建监听地址
    std::string server_address = listen_address + ":" + std::to_string(port);

    // 创建服务实现
    pImpl_->service_impl_ =
        std::make_unique<ClientCallbackServiceImpl>(pImpl_->message_callback_);

    // 构建服务器
    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(pImpl_->service_impl_.get());

    // 启用健康检查和反射（可选）
    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();

    // 构建并启动服务器
    pImpl_->server_ = builder.BuildAndStart();

    if (!pImpl_->server_) {
      return Status(std::make_error_code(std::errc::address_not_available),
                    "Failed to start gRPC callback server");
    }

    pImpl_->running_ = true;

    // 在单独线程中运行服务器
    pImpl_->server_thread_ = std::thread([this]() {
      try {

        std::cout << "Client callback server listening on "
                  << pImpl_->listen_address_ << ":" << pImpl_->listen_port_
                  << std::endl;
        pImpl_->server_->Wait();
      } catch (const std::exception &e) {
        std::cerr << "❌ Error in server thread: " << e.what() << std::endl;
      }
    });

    return Status();
  } catch (const std::exception &e) {
    return Status(std::make_error_code(std::errc::operation_not_supported),
                  std::string("Failed to start callback server: ") + e.what());
  }
}

Status
ClientCallbackServer::StartWithAutoPort(const std::string &listen_address,
                                        int &assigned_port) {
  if (pImpl_->running_) {
    return Status(std::make_error_code(std::errc::operation_not_permitted),
                  "Server is already running");
  }

  try {
    pImpl_->listen_address_ = listen_address;
    pImpl_->listen_port_ = 0; // 使用0让系统自动分配

    // 构建监听地址
    std::string server_address = listen_address + ":0";

    // 创建服务实现
    pImpl_->service_impl_ =
        std::make_unique<ClientCallbackServiceImpl>(pImpl_->message_callback_);

    // 构建服务器
    grpc::ServerBuilder builder;
    int selected_port = 0;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials(),
                             &selected_port);
    builder.RegisterService(pImpl_->service_impl_.get());

    // 启用健康检查和反射（可选）
    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();

    // 构建并启动服务器
    pImpl_->server_ = builder.BuildAndStart();

    if (!pImpl_->server_) {
      return Status(std::make_error_code(std::errc::address_not_available),
                    "Failed to start gRPC callback server");
    }

    // 获取实际分配的端口
    assigned_port = selected_port;
    pImpl_->listen_port_ = assigned_port;
    pImpl_->running_ = true;

    // 在单独线程中运行服务器
    pImpl_->server_thread_ = std::thread([this]() {
      std::cout << "Client callback server listening on "
                << pImpl_->listen_address_ << ":" << pImpl_->listen_port_
                << std::endl;
      pImpl_->server_->Wait();
    });

    return Status();
  } catch (const std::exception &e) {
    return Status(std::make_error_code(std::errc::operation_not_supported),
                  std::string("Failed to start callback server: ") + e.what());
  }
}

void ClientCallbackServer::Stop() { pImpl_->Stop(); }

bool ClientCallbackServer::IsRunning() const { return pImpl_->running_; }

std::string ClientCallbackServer::GetListenAddress() const {
  return pImpl_->listen_address_;
}

int ClientCallbackServer::GetListenPort() const { return pImpl_->listen_port_; }

void ClientCallbackServer::SetSubscriptionMessageCallback(
    SubscriptionMessageCallback callback) {
  if (pImpl_->running_) {
    throw std::runtime_error("Cannot set callback while server is running. "
                             "Please set callback before starting the server.");
  }

  pImpl_->message_callback_ = callback;
}

std::string ClientCallbackServer::GetClientEndpoint() const {
  if (pImpl_->listen_address_.empty() || pImpl_->listen_port_ == 0) {
    return "";
  }
  return pImpl_->listen_address_ + ":" + std::to_string(pImpl_->listen_port_);
}

// =============================================================================
// 便捷工厂函数实现
// =============================================================================

namespace humanoid_robot {
namespace clientSDK {
namespace robot {
std::unique_ptr<ClientCallbackServer>
CreateCallbackServer(const std::string &listen_address, Status &status,
                     int port, SubscriptionMessageCallback message_callback) {
  auto server = std::make_unique<ClientCallbackServer>();

  // 设置回调函数
  if (message_callback) {
    server->SetSubscriptionMessageCallback(message_callback);
  }

  // 启动服务器
  if (port == 0) {
    int assigned_port;
    status = server->StartWithAutoPort(listen_address, assigned_port);
  } else {
    status = server->Start(listen_address, port);
  }

  return server;
}
} // namespace robot
} // namespace clientSDK
} // namespace humanoid_robot
