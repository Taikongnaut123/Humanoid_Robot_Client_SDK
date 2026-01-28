/**
 * Copyright (c) 2025 Humanoid Robot, Inc. All rights reserved.
 *
 * Interfaces gRPC Client for Robot SDK
 * Provides async and sync interfaces for InterfaceService operations
 */

#ifndef HUMANOID_ROBOT_INTERFACES_CLIENT_H
#define HUMANOID_ROBOT_INTERFACES_CLIENT_H

#include <grpcpp/grpcpp.h>

#include <functional>
#include <future>
#include <memory>
#include <string>
#include <vector>

#include "interfaces/interfaces_grpc.grpc.pb.h"
#include "interfaces/interfaces_request_response.pb.h"
#include "robot/common/status.h"

namespace humanoid_robot {
namespace clientSDK {
namespace robot {
using Status = humanoid_robot::clientSDK::common::Status;

// Forward declarations for async operation results
template <typename T> using AsyncResult = std::future<Status>;

template <typename T>
using AsyncCallback = std::function<void(const Status &, const T &)>;

/**
 * InterfacesClient - gRPC client for InterfaceService
 *
 * This client provides both synchronous and asynchronous methods for all
 * InterfaceService operations defined in interfaces_grpc.proto
 */
class InterfacesClient : public std::enable_shared_from_this<InterfacesClient> {
public:
  // Constructor and destructor
  InterfacesClient();
  ~InterfacesClient();

  // Connection management
  Status Connect(const std::string &server_address, int port);
  Status Connect(const std::string &target);
  void Disconnect();
  bool IsConnected() const;

  // =================================================================
  // Synchronous Methods
  // =================================================================

  /**
   * Send a message
   * @param request The send request
   * @param response The send response (output)
   * @param timeout_ms Timeout in milliseconds (default: 5000)
   * @return Status of the operation
   */
  Status Send(std::unique_ptr<::grpc::ClientReaderWriter<
                  ::humanoid_robot::PB::interfaces::SendRequest,
                  ::humanoid_robot::PB::interfaces::SendResponse>> &readWriter,
              std::unique_ptr<grpc::ClientContext> &context,
              int64_t timeout_ms = 5000);

  /**
   * Query resources
   * @param request The query request
   * @param response The query response (output)
   * @param timeout_ms Timeout in milliseconds (default: 5000)
   * @return Status of the operation
   */
  Status Query(const humanoid_robot::PB::interfaces::QueryRequest &request,
               humanoid_robot::PB::interfaces::QueryResponse &response,
               int64_t timeout_ms = 5000);

  /**
   * Action resources with streaming response
   * @param request The action request
   * @param reader The client reader for streaming responses (output)
   * @param context The client context (must remain valid during stream
   * lifetime)
   * @return Status of the operation
   */
  Status Action(const humanoid_robot::PB::interfaces::ActionRequest &request,
                std::unique_ptr<::grpc::ClientReader<
                    ::humanoid_robot::PB::interfaces::ActionResponse>> &reader,
                grpc::ClientContext &context);

  /**
   * Unsubscribe from a subscription
   * @param request The unsubscribe request
   * @param response The unsubscribe response (output)
   * @param timeout_ms Timeout in milliseconds (default: 5000)
   * @return Status of the operation
   */
  Status
  Unsubscribe(const humanoid_robot::PB::interfaces::UnsubscribeRequest &request,
              humanoid_robot::PB::interfaces::UnsubscribeResponse &response,
              int64_t timeout_ms = 5000);

  // =================================================================
  // Asynchronous Methods
  // =================================================================

  /**
   * Async send - returns immediately with a future
   */
  // AsyncResult<humanoid_robot::PB::interfaces::SendResponse> SendAsync(
  //     const humanoid_robot::PB::interfaces::SendRequest &request,
  //     int64_t timeout_ms = 5000);

  // /**
  //  * Async send with callback
  //  */
  // void SendAsync(
  //     const humanoid_robot::PB::interfaces::SendRequest &request,
  //     AsyncCallback<humanoid_robot::PB::interfaces::SendResponse> callback,
  //     int64_t timeout_ms = 5000);

  /**
   * Async query - returns immediately with a future
   */
  AsyncResult<humanoid_robot::PB::interfaces::QueryResponse>
  QueryAsync(const humanoid_robot::PB::interfaces::QueryRequest &request,
             int64_t timeout_ms = 5000);

  /**
   * Async query with callback
   */
  void QueryAsync(
      const humanoid_robot::PB::interfaces::QueryRequest &request,
      AsyncCallback<humanoid_robot::PB::interfaces::QueryResponse> callback,
      int64_t timeout_ms = 5000);

  // =================================================================
  // Streaming Methods
  // =================================================================

  /**
   * Subscribe to events (streaming response) - Legacy方式
   * @param request The subscription request
   * @param callback Called for each received event
   * @param timeout_ms Timeout for the subscription (0 = no timeout)
   * @return Status of the operation (returns when stream ends or errors)
   */
  Status
  Subscribe(const humanoid_robot::PB::interfaces::SubscribeRequest &request,
            humanoid_robot::PB::interfaces::SubscribeResponse &response,
            int64_t timeout_ms = 0);

  // =================================================================
  // Utility Methods
  // =================================================================

  /**
   * Get the current gRPC channel state
   */
  grpc_connectivity_state GetChannelState(bool try_to_connect = false);

  /**
   * Wait for the channel to be ready
   * @param timeout_ms Maximum time to wait
   * @return True if channel became ready within timeout
   */
  bool WaitForChannelReady(int64_t timeout_ms = 5000);

private:
  // Private implementation details
  class InterfacesClientImpl;
  std::unique_ptr<InterfacesClientImpl> pImpl_;

  // Disable copy and assignment
  InterfacesClient(const InterfacesClient &) = delete;
  InterfacesClient &operator=(const InterfacesClient &) = delete;

  // Helper methods
  Status ConvertGrpcStatus(const grpc::Status &grpc_status);
  std::chrono::system_clock::time_point GetDeadline(int64_t timeout_ms);
};

// =================================================================
// Convenience Functions
// =================================================================

/**
 * Create a quick interfaces client connected to a server
 * @param server_address Server address (e.g., "localhost")
 * @param port Server port
 * @param client Output client instance
 * @return Status of the connection
 */
Status CreateInterfacesClientLegacy(const std::string &server_address, int port,
                                    std::unique_ptr<InterfacesClient> &client);

} // namespace robot

// Factory functions for creating clients
namespace factory {
using Status = humanoid_robot::clientSDK::common::Status;
/**
 * Create and connect interfaces client (推荐用于异步操作)
 * @param server_address Server address (e.g., "localhost")
 * @param port Server port (e.g., 50051)
 * @param client Output client instance (shared_ptr for async safety)
 * @return Status of the connection
 */
Status CreateInterfacesClient(const std::string &server_address, int port,
                              std::shared_ptr<robot::InterfacesClient> &client);

/**
 * Create a quick interfaces client with target string (推荐用于异步操作)
 * @param target Target string (e.g., "localhost:50051")
 * @param client Output client instance (shared_ptr for async safety)
 * @return Status of the connection
 */
Status CreateInterfacesClient(const std::string &target,
                              std::shared_ptr<robot::InterfacesClient> &client);
} // namespace factory
} // namespace clientSDK
} // namespace humanoid_robot

#endif // HUMANOID_ROBOT_INTERFACES_CLIENT_H
