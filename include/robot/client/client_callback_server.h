/**
 * Copyright (c) 2025 Humanoid Robot, Inc. All rights reserved.
 *
 * Client Callback Server for receiving subscription notifications from server
 */

#ifndef HUMANOID_ROBOT_CLIENT_CALLBACK_SERVER_H
#define HUMANOID_ROBOT_CLIENT_CALLBACK_SERVER_H

#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <thread>

#include "interfaces/interfaces_callback.grpc.pb.h"
#include "robot/common/status.h"
#include <grpcpp/grpcpp.h>

using humanoid_robot::konka_sdk::common::Status;

namespace humanoid_robot {
namespace konka_sdk {
namespace robot {
// 回调函数类型定义
using SubscriptionMessageCallback =
    std::function<void(const humanoid_robot::PB::interfaces::Notification &)>;

/**
 * ClientCallbackServer - 客户端回调服务器
 *
 * 用于接收来自 Interfaces-Server 的主动推送消息
 * 当服务端有订阅消息时，会调用此服务器的接口推送给客户端
 */
class ClientCallbackServer {
public:
  ClientCallbackServer();
  ~ClientCallbackServer();

  // =================================================================
  // 服务器生命周期管理
  // =================================================================

  /**
   * 启动回调服务器
   * @param listen_address 监听地址 (e.g., "0.0.0.0", "localhost")
   * @param port 监听端口
   * @return 启动状态
   */
  Status Start(const std::string &listen_address, int port);

  /**
   * 启动回调服务器（自动分配端口）
   * @param listen_address 监听地址
   * @param assigned_port 输出分配的端口号
   * @return 启动状态
   */
  Status StartWithAutoPort(const std::string &listen_address,
                           int &assigned_port);

  /**
   * 停止回调服务器
   */
  void Stop();

  /**
   * 检查服务器是否正在运行
   */
  bool IsRunning() const;

  /**
   * 获取服务器监听地址
   */
  std::string GetListenAddress() const;

  /**
   * 获取服务器监听端口
   */
  int GetListenPort() const;

  // =================================================================
  // 回调函数注册
  // =================================================================

  /**
   * 注册订阅消息回调
   * @param callback 消息回调函数
   */
  void SetSubscriptionMessageCallback(SubscriptionMessageCallback callback);

  // =================================================================
  // 便捷方法
  // =================================================================

  /**
   * 获取客户端服务端点（用于订阅时告知服务端）
   * @return 格式为 "address:port" 的端点字符串
   */
  std::string GetClientEndpoint() const;

private:
  // Private implementation
  class Impl;
  std::unique_ptr<Impl> pImpl_;

  // Disable copy and assignment
  ClientCallbackServer(const ClientCallbackServer &) = delete;
  ClientCallbackServer &operator=(const ClientCallbackServer &) = delete;
};

// =================================================================
// 便捷工厂函数
// =================================================================

/**
 * 创建并启动客户端回调服务器
 * @param listen_address 监听地址
 * @param status 输出状态
 * @param port 监听端口（0表示自动分配）
 * @param message_callback 消息回调
 * @return 服务器实例
 */
std::unique_ptr<ClientCallbackServer>
CreateCallbackServer(const std::string &listen_address, Status &status,
                     int port = 0,
                     SubscriptionMessageCallback message_callback = nullptr);

} // namespace robot
} // namespace konka_sdk
} // namespace humanoid_robot

#endif // HUMANOID_ROBOT_CLIENT_CALLBACK_SERVER_H
