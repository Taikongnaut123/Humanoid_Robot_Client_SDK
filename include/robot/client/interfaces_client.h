/**
 * Copyright (c) 2025 Humanoid Robot, Inc. All rights reserved.
 *
 * Interfaces gRPC Client for Robot SDK
 * Provides async and sync interfaces for InterfaceService operations
 */

#ifndef HUMANOID_ROBOT_INTERFACES_CLIENT_H
#define HUMANOID_ROBOT_INTERFACES_CLIENT_H

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <future>

#include <grpcpp/grpcpp.h>
#include "robot/common/status.h"
#include "interfaces/interfaces_grpc.grpc.pb.h"
#include "interfaces/interfaces_request_response.pb.h"

using humanoid_robot::clientSDK::common::Status;

namespace humanoid_robot
{
    namespace clientSDK
    {
        namespace robot
        {
            // Forward declarations for async operation results
            template <typename T>
            using AsyncResult = std::future<Status>;

            template <typename T>
            using AsyncCallback = std::function<void(const Status &, const T &)>;

            /**
             * InterfacesClient - gRPC client for InterfaceService
             *
             * This client provides both synchronous and asynchronous methods for all
             * InterfaceService operations defined in interfaces_grpc.proto
             */
            class InterfacesClient : public std::enable_shared_from_this<InterfacesClient>
            {
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
                 * Create a new resource
                 * @param request The create request
                 * @param response The create response (output)
                 * @param timeout_ms Timeout in milliseconds (default: 5000)
                 * @return Status of the operation
                 */
                Status Create(
                    const interfaces::CreateRequest &request,
                    interfaces::CreateResponse &response,
                    int64_t timeout_ms = 5000);

                /**
                 * Send a message
                 * @param request The send request
                 * @param response The send response (output)
                 * @param timeout_ms Timeout in milliseconds (default: 5000)
                 * @return Status of the operation
                 */
                Status Send(
                    const interfaces::SendRequest &request,
                    interfaces::SendResponse &response,
                    int64_t timeout_ms = 5000);

                /**
                 * Delete a resource
                 * @param request The delete request
                 * @param response The delete response (output)
                 * @param timeout_ms Timeout in milliseconds (default: 5000)
                 * @return Status of the operation
                 */
                Status Delete(
                    const interfaces::DeleteRequest &request,
                    interfaces::DeleteResponse &response,
                    int64_t timeout_ms = 5000);

                /**
                 * Query resources
                 * @param request The query request
                 * @param response The query response (output)
                 * @param timeout_ms Timeout in milliseconds (default: 5000)
                 * @return Status of the operation
                 */
                Status Query(
                    const interfaces::QueryRequest &request,
                    interfaces::QueryResponse &response,
                    int64_t timeout_ms = 5000);

                /**
                 * Unsubscribe from a subscription
                 * @param request The unsubscribe request
                 * @param response The unsubscribe response (output)
                 * @param timeout_ms Timeout in milliseconds (default: 5000)
                 * @return Status of the operation
                 */
                Status Unsubscribe(
                    const interfaces::UnsubscribeRequest &request,
                    interfaces::UnsubscribeResponse &response,
                    int64_t timeout_ms = 5000);

                // =================================================================
                // Asynchronous Methods
                // =================================================================

                /**
                 * Async create - returns immediately with a future
                 */
                AsyncResult<interfaces::CreateResponse> CreateAsync(
                    const interfaces::CreateRequest &request,
                    int64_t timeout_ms = 5000);

                /**
                 * Async create with callback
                 */
                void CreateAsync(
                    const interfaces::CreateRequest &request,
                    AsyncCallback<interfaces::CreateResponse> callback,
                    int64_t timeout_ms = 5000);

                /**
                 * Async send - returns immediately with a future
                 */
                AsyncResult<interfaces::SendResponse> SendAsync(
                    const interfaces::SendRequest &request,
                    int64_t timeout_ms = 5000);

                /**
                 * Async send with callback
                 */
                void SendAsync(
                    const interfaces::SendRequest &request,
                    AsyncCallback<interfaces::SendResponse> callback,
                    int64_t timeout_ms = 5000);

                /**
                 * Async query - returns immediately with a future
                 */
                AsyncResult<interfaces::QueryResponse> QueryAsync(
                    const interfaces::QueryRequest &request,
                    int64_t timeout_ms = 5000);

                /**
                 * Async query with callback
                 */
                void QueryAsync(
                    const interfaces::QueryRequest &request,
                    AsyncCallback<interfaces::QueryResponse> callback,
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
                Status Subscribe(
                    const interfaces::SubscribeRequest &request,
                    interfaces::SubscribeResponse &response,
                    int64_t timeout_ms = 0);

                /**
                 * 查询订阅状态
                 * @param subscription_id 订阅ID
                 * @param subscription_info 输出订阅信息
                 * @return 查询状态
                 */
                Status GetSubscriptionStatus(
                    const std::string &subscription_id,
                    base_types::Dictionary &subscription_info);

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
                class Impl;
                std::unique_ptr<Impl> pImpl_;

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
            Status CreateInterfacesClientLegacy(
                const std::string &server_address,
                int port,
                std::unique_ptr<InterfacesClient> &client);

        } // namespace robot

        // Factory functions for creating clients
        namespace factory
        {
            /**
             * Create and connect interfaces client (推荐用于异步操作)
             * @param server_address Server address (e.g., "localhost")
             * @param port Server port (e.g., 50051)
             * @param client Output client instance (shared_ptr for async safety)
             * @return Status of the connection
             */
            Status CreateInterfacesClient(
                const std::string &server_address,
                int port,
                std::shared_ptr<robot::InterfacesClient> &client);

            /**
             * Create a quick interfaces client with target string (推荐用于异步操作)
             * @param target Target string (e.g., "localhost:50051")
             * @param client Output client instance (shared_ptr for async safety)
             * @return Status of the connection
             */
            Status CreateInterfacesClient(
                const std::string &target,
                std::shared_ptr<robot::InterfacesClient> &client);
        } // namespace factory
    } // namespace clientSDK
} // namespace humanoid_robot

#endif // HUMANOID_ROBOT_INTERFACES_CLIENT_H
