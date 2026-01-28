/**
 * Copyright (c) 2025 Humanoid Robot, Inc. All rights reserved.
 *
 * Implementation of InterfacesClient for Robot SDK
 */

#include "robot/client/interfaces_client.h"

#include <grpcpp/grpcpp.h>

#include <chrono>
#include <ctime>
#include <iostream>
#include <thread>

#include "robot/common/errorCode.h"

using namespace humanoid_robot::clientSDK::robot;
using namespace humanoid_robot::clientSDK::common;
using namespace humanoid_robot::PB::interfaces;

// Private implementation class
class InterfacesClient::InterfacesClientImpl {
public:
  std::shared_ptr<grpc::Channel> channel_;
  std::unique_ptr<InterfaceService::Stub> stub_;
  std::string target_;
  bool connected_;

  InterfacesClientImpl() : connected_(false) {}

  ~InterfacesClientImpl() {
    if (connected_) {
      // Graceful shutdown could be implemented here
    }
  }
};

// =================================================================
// Constructor and Destructor
// =================================================================

InterfacesClient::InterfacesClient()
    : pImpl_(std::make_unique<InterfacesClientImpl>()) {}

InterfacesClient::~InterfacesClient() = default;

// =================================================================
// Connection Management
// =================================================================

Status InterfacesClient::Connect(const std::string &server_address, int port) {
  std::string target = server_address + ":" + std::to_string(port);
  return Connect(target);
}

Status InterfacesClient::Connect(const std::string &target) {
  try {
    pImpl_->target_ = target;

    // Create gRPC channel with default credentials
    grpc::ChannelArguments args;
    args.SetMaxReceiveMessageSize(100 * 1024 * 1024); // 100MB max message size
    args.SetMaxSendMessageSize(100 * 1024 * 1024);

    pImpl_->channel_ = grpc::CreateCustomChannel(
        target, grpc::InsecureChannelCredentials(), args);

    if (!pImpl_->channel_) {
      return Status(std::make_error_code(std::errc::connection_refused),
                    "Failed to create gRPC channel");
    }

    // Create service stub
    pImpl_->stub_ = InterfaceService::NewStub(pImpl_->channel_);

    if (!pImpl_->stub_) {
      return Status(std::make_error_code(std::errc::connection_refused),
                    "Failed to create service stub");
    }

    pImpl_->connected_ = true;

    if (!WaitForChannelReady(5000)) {
      return Status(std::make_error_code(std::errc::connection_refused),
                    "Failed to create gRPC channel");
    }

    return Status();
  } catch (const std::exception &e) {
    return Status(std::make_error_code(std::errc::connection_refused),
                  std::string("Connection failed: ") + e.what());
  }
}

void InterfacesClient::Disconnect() {
  pImpl_->stub_.reset();
  pImpl_->channel_.reset();
  pImpl_->connected_ = false;
}

bool InterfacesClient::IsConnected() const {
  return pImpl_->connected_ && pImpl_->channel_ && pImpl_->stub_;
}

// =================================================================
// Synchronous Methods
// =================================================================

Status InterfacesClient::Send(
    std::unique_ptr<::grpc::ClientReaderWriter<
        ::humanoid_robot::PB::interfaces::SendRequest,
        ::humanoid_robot::PB::interfaces::SendResponse>> &readWriter,
    std::unique_ptr<grpc::ClientContext> &context, int64_t timeout_ms) {
  if (!IsConnected()) {
    return Status(std::make_error_code(std::errc::not_connected),
                  "Client not connected");
  }

  // Allocate context on heap and pass ownership to caller
  context = std::make_unique<grpc::ClientContext>();
  context->set_deadline(GetDeadline(timeout_ms));

  readWriter = pImpl_->stub_->Send(context.get());
  if (!readWriter) {
    return Status(std::make_error_code(std::errc::io_error),
                  "Failed to create send stream");
  }
  return Status();
}

Status InterfacesClient::Query(
    const humanoid_robot::PB::interfaces::QueryRequest &request,
    humanoid_robot::PB::interfaces::QueryResponse &response,
    int64_t timeout_ms) {
  if (!IsConnected()) {
    return Status(std::make_error_code(std::errc::not_connected),
                  "Client not connected");
  }

  grpc::ClientContext context;
  context.set_deadline(GetDeadline(timeout_ms));

  grpc::Status status = pImpl_->stub_->Query(&context, request, &response);
  return ConvertGrpcStatus(status);
}

Status InterfacesClient::Action(
    const humanoid_robot::PB::interfaces::ActionRequest &request,
    std::unique_ptr<
        ::grpc::ClientReader<::humanoid_robot::PB::interfaces::ActionResponse>>
        &reader,
    grpc::ClientContext &context) {
  if (!IsConnected()) {
    return Status(std::make_error_code(std::errc::not_connected),
                  "Client not connected");
  }

  reader = pImpl_->stub_->Action(&context, request);

  if (!reader) {
    return Status(std::make_error_code(std::errc::io_error),
                  "Failed to create action stream");
  }

  return Status(); // 成功
}

Status InterfacesClient::Unsubscribe(
    const humanoid_robot::PB::interfaces::UnsubscribeRequest &request,
    humanoid_robot::PB::interfaces::UnsubscribeResponse &response,
    int64_t timeout_ms) {
  if (!IsConnected()) {
    return Status(std::make_error_code(std::errc::not_connected),
                  "Client not connected");
  }

  grpc::ClientContext context;
  context.set_deadline(GetDeadline(timeout_ms));

  grpc::Status status =
      pImpl_->stub_->Unsubscribe(&context, request, &response);
  return ConvertGrpcStatus(status);
}

// =================================================================
// Asynchronous Methods (Future-based)
// =================================================================

// AsyncResult<humanoid_robot::PB::interfaces::SendResponse>
// InterfacesClient::SendAsync(
//     const humanoid_robot::PB::interfaces::SendRequest &request,
//     int64_t timeout_ms)
// {
//     auto promise = std::make_shared<std::promise<Status>>();
//     auto future = promise->get_future();

//     SendAsync(request, [promise](const Status &status, const
//     humanoid_robot::PB::interfaces::SendResponse &response)
//               { promise->set_value(status); }, timeout_ms);

//     return future;
// }

// void InterfacesClient::SendAsync(
//     const humanoid_robot::PB::interfaces::SendRequest &request,
//     AsyncCallback<humanoid_robot::PB::interfaces::SendResponse> callback,
//     int64_t timeout_ms)
// {
//     std::weak_ptr<InterfacesClient> weak_self = shared_from_this();

//     std::thread([weak_self, request, callback, timeout_ms]()
//                 {
//                 if (auto self = weak_self.lock()) {
//                     SendResponse response;
//                     auto status = self->Send(request, response, timeout_ms);
//                     callback(status, response);
//                 } else {
//                     SendResponse response;
//                     Status error_status(
//                         std::make_error_code(std::errc::operation_canceled),
//                         "Client object has been destroyed");
//                     callback(error_status, response);
//                 } })
//         .detach();
// }

AsyncResult<humanoid_robot::PB::interfaces::QueryResponse>
InterfacesClient::QueryAsync(
    const humanoid_robot::PB::interfaces::QueryRequest &request,
    int64_t timeout_ms) {
  auto promise = std::make_shared<std::promise<Status>>();
  auto future = promise->get_future();

  QueryAsync(
      request,
      [promise](const Status &status, const QueryResponse &response) {
        promise->set_value(status);
      },
      timeout_ms);

  return future;
}

void InterfacesClient::QueryAsync(
    const humanoid_robot::PB::interfaces::QueryRequest &request,
    AsyncCallback<humanoid_robot::PB::interfaces::QueryResponse> callback,
    int64_t timeout_ms) {
  std::weak_ptr<InterfacesClient> weak_self = shared_from_this();

  std::thread([weak_self, request, callback, timeout_ms]() {
    if (auto self = weak_self.lock()) {
      QueryResponse response;
      auto status = self->Query(request, response, timeout_ms);
      callback(status, response);
    } else {
      QueryResponse response;
      Status error_status(std::make_error_code(std::errc::operation_canceled),
                          "Client object has been destroyed");
      callback(error_status, response);
    }
  }).detach();
}

// =================================================================
// Streaming Methods
// =================================================================

Status InterfacesClient::Subscribe(
    const humanoid_robot::PB::interfaces::SubscribeRequest &request,
    humanoid_robot::PB::interfaces::SubscribeResponse &response,
    int64_t timeout_ms) {
  if (!IsConnected()) {
    return Status(std::make_error_code(std::errc::not_connected),
                  "Client not connected");
  }

  grpc::ClientContext context;
  if (timeout_ms > 0) {
    context.set_deadline(GetDeadline(timeout_ms));
  }

  auto status = pImpl_->stub_->Subscribe(&context, request, &response);
  return ConvertGrpcStatus(status);
}

// =================================================================
// Utility Methods
// =================================================================

grpc_connectivity_state InterfacesClient::GetChannelState(bool try_to_connect) {
  if (!pImpl_->channel_) {
    return GRPC_CHANNEL_SHUTDOWN;
  }
  return pImpl_->channel_->GetState(try_to_connect);
}

bool InterfacesClient::WaitForChannelReady(int64_t timeout_ms) {
  if (!pImpl_->channel_) {
    return false;
  }

  auto deadline = GetDeadline(timeout_ms);
  auto state = pImpl_->channel_->GetState(true);

  while (state != GRPC_CHANNEL_READY &&
         std::chrono::system_clock::now() < deadline) {
    state = pImpl_->channel_->GetState(false);

    // print remaining time in seconds
    std::cout << "Wait for channel ready, remaining time: "
              << int((deadline - std::chrono::system_clock::now()).count() /
                     1e9)
              << " s" << std::endl;
    // sleep 1 s
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  return state == GRPC_CHANNEL_READY;
}

// =================================================================
// Private Helper Methods
// =================================================================

Status InterfacesClient::ConvertGrpcStatus(const grpc::Status &grpc_status) {
  if (grpc_status.ok()) {
    return Status();
  }

  std::error_code ec;
  switch (grpc_status.error_code()) {
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

std::chrono::system_clock::time_point
InterfacesClient::GetDeadline(int64_t timeout_ms) {
  return std::chrono::system_clock::now() +
         std::chrono::milliseconds(timeout_ms);
}

// =================================================================
// Convenience Functions
// =================================================================

// Factory functions implementation
namespace humanoid_robot::clientSDK::factory {
// Preferred shared_ptr versions for async safety
Status
CreateInterfacesClient(const std::string &server_address, int port,
                       std::shared_ptr<robot::InterfacesClient> &client) {
  client =
      std::shared_ptr<robot::InterfacesClient>(new robot::InterfacesClient());
  return client->Connect(server_address, port);
}

Status
CreateInterfacesClient(const std::string &target,
                       std::shared_ptr<robot::InterfacesClient> &client) {
  client =
      std::shared_ptr<robot::InterfacesClient>(new robot::InterfacesClient());
  return client->Connect(target);
}
} // namespace humanoid_robot::clientSDK::factory