# Client-SDK

Client-SDK 是一个用于连接和操作 Humanoid Robot 系统的 C++ 客户端库。它提供了一个简单易用的接口来与 Interfaces 服务进行通信，支持多线程回调架构和实时推送通知。

## 核心特性

- **完整的gRPC客户端实现** - 支持所有Interfaces服务操作
- **多线程回调架构** - 非阻塞的服务器推送消息接收
- **持久化订阅支持** - 实时事件通知和状态更新
- **同步/异步操作** - 灵活的调用模式选择
- **线程安全设计** - 适用于多线程应用环境
- **自动端口分配** - 简化回调服务器部署
- **完善的错误处理** - 详细的状态码和错误信息

## 目录结构

```
Client-SDK/
├── include/robot/
│   ├── client/
│   │   ├── interfaces_client.h          # 主要客户端接口
│   │   └── client_callback_server.h     # 回调服务器接口
│   ├── common/
│   │   ├── status.h                     # 状态类定义
│   │   ├── errorCode.h                  # 错误码定义
│   │   ├── marco.h                      # 宏定义
│   │   └── success_condition.h          # 成功条件定义
│   └── v1/                              # 版本1接口（预留）
├── source/robot/
│   ├── client/
│   │   ├── interfaces_client.cpp        # 客户端实现
│   │   ├── client_callback_server.cpp   # 回调服务器实现
│   │   └── CMakeLists.txt               # 客户端构建配置
│   ├── common/                          # 通用组件实现
│   ├── v1/                              # 版本1实现（预留）
│   └── CMakeLists.txt                   # 源码构建配置
├── example/
│   ├── interfaces_client_example.cpp    # 使用示例
│   └── CMakeLists.txt                   # 示例构建配置
├── docs/
│   └── requirements_specification.md    # 需求规范文档
├── cmake/                               # CMake 辅助文件（预留）
├── CMakeLists.txt                       # 主构建配置
├── DESIGN_DOCUMENT.md                   # 设计文档
├── .gitignore                           # Git 忽略文件
└── README.md                            # 本文档
```

## 构建

```bash
# 在项目根目录下
mkdir build && cd build
cmake ..
make
```

构建产物将输出到 `bin/linux_x64/release/` 目录。

## 功能概述

Client-SDK 提供以下核心功能：

### InterfacesClient - 客户端接口
#### 同步操作
- `Send` - 发送消息到服务器
- `Query` - 查询资源信息
- `Action` - 执行动作操作（支持流式响应）
- `Subscribe` - 订阅事件流
- `Unsubscribe` - 取消订阅

#### 异步操作
- Future-based 异步调用
- Callback-based 异步调用
- 非阻塞操作
- 错误处理

#### 流式操作
- **Action** - 动作操作（服务器流）
- **Subscribe** - 事件订阅（服务器流）
- 实时响应处理
- 流错误处理
- 超时控制

### ClientCallbackServer - 回调服务器
- **主动推送接收** - 接收来自服务器的主动推送消息
- **订阅通知处理** - 处理订阅事件的实时通知
- **状态变化监听** - 监听订阅状态的变化
- **错误处理** - 处理订阅过程中的错误
- **生命周期管理** - 启动、停止和状态监控

## 使用示例

### 基本连接

```cpp
#include "robot/client/interfaces_client.h"

using namespace humanoid_robot::clientSDK::robot;

// 方法1: 使用便捷函数（返回unique_ptr）
std::unique_ptr<InterfacesClient> client;
auto status = CreateInterfacesClientLegacy("localhost", 50051, client);

// 方法2: 使用工厂函数（返回shared_ptr，推荐用于异步操作）
std::shared_ptr<InterfacesClient> shared_client;
auto status2 = humanoid_robot::clientSDK::factory::CreateInterfacesClient("localhost", 50051, shared_client);

// 方法3: 手动创建和连接
auto manual_client = std::make_unique<InterfacesClient>();
auto status3 = manual_client->Connect("localhost", 50051);
```

### 发送消息

```cpp
interfaces::SendRequest request;

// 设置输入数据（使用 Dictionary 结构）
auto* input_dict = request.mutable_input();
auto* input_kv = input_dict->mutable_keyvaluelist();

// 添加消息ID
base_types::Variant message_id_variant;
message_id_variant.set_type(base_types::Variant::KStringValue);
message_id_variant.set_stringvalue("msg-001");
input_kv->insert(std::make_pair("messageId", message_id_variant));

// 添加消息内容
base_types::Variant content_variant;
content_variant.set_type(base_types::Variant::KStringValue);
content_variant.set_stringvalue("Hello, Robot!");
input_kv->insert(std::make_pair("content", content_variant));

// 设置参数（使用 Dictionary 结构）
auto* params_dict = request.mutable_params();
auto* params_kv = params_dict->mutable_keyvaluelist();

// 添加超时参数
base_types::Variant timeout_variant;
timeout_variant.set_type(base_types::Variant::KInt32Value);
timeout_variant.set_int32value(30);
params_kv->insert(std::make_pair("timeout", timeout_variant));

// 添加关联ID
base_types::Variant correlation_variant;
correlation_variant.set_type(base_types::Variant::KStringValue);
correlation_variant.set_stringvalue("send-001");
params_kv->insert(std::make_pair("correlationId", correlation_variant));

interfaces::SendResponse response;
auto status = client->Send(request, response, 5000);

if (status) {
    std::cout << "Message sent successfully!" << std::endl;
    
    // 检查返回状态
    if (response.has_ret()) {
        std::cout << "Return code: " << response.ret().code() << std::endl;
        std::cout << "Return message: " << response.ret().message() << std::endl;
    }
    
    // 处理输出数据
    if (response.has_output()) {
        const auto& output_kv = response.output().keyvaluelist();
        for (const auto& [key, value] : output_kv) {
            std::cout << "Output " << key << ": ";
            if (value.type() == base_types::Variant::KStringValue) {
                std::cout << value.stringvalue() << std::endl;
            }
            // 处理其他类型...
        }
    }
}
```

### 查询操作

```cpp
interfaces::QueryRequest request;

// 设置输入数据
auto* input_dict = request.mutable_input();
auto* input_kv = input_dict->mutable_keyvaluelist();

// 添加查询ID
base_types::Variant query_id_variant;
query_id_variant.set_type(base_types::Variant::KStringValue);
query_id_variant.set_stringvalue("find_all");
input_kv->insert(std::make_pair("queryId", query_id_variant));

// 设置参数
auto* params_dict = request.mutable_params();
auto* params_kv = params_dict->mutable_keyvaluelist();

// 添加限制数量
base_types::Variant limit_variant;
limit_variant.set_type(base_types::Variant::KInt32Value);
limit_variant.set_int32value(10);
params_kv->insert(std::make_pair("limit", limit_variant));

// 添加偏移量
base_types::Variant offset_variant;
offset_variant.set_type(base_types::Variant::KInt32Value);
offset_variant.set_int32value(0);
params_kv->insert(std::make_pair("offset", offset_variant));

interfaces::QueryResponse response;
auto status = client->Query(request, response, 5000);

if (status) {
    std::cout << "Query executed successfully!" << std::endl;
    
    // 检查返回状态
    if (response.has_ret()) {
        std::cout << "Return code: " << response.ret().code() << std::endl;
        std::cout << "Return message: " << response.ret().message() << std::endl;
    }
    
    // 处理查询结果
    if (response.has_output()) {
        const auto& output_kv = response.output().keyvaluelist();
        
        // 查找总数
        auto total_it = output_kv.find("totalCount");
        if (total_it != output_kv.end() && total_it->second.type() == base_types::Variant::KInt32Value) {
            std::cout << "Total count: " << total_it->second.int32value() << std::endl;
        }
        
        // 查找结果数量
        auto count_it = output_kv.find("resultCount");
        if (count_it != output_kv.end() && count_it->second.type() == base_types::Variant::KInt32Value) {
            std::cout << "Returned items: " << count_it->second.int32value() << std::endl;
        }
    }
}
```

### 执行动作操作（流式响应）

```cpp
interfaces::ActionRequest request;

// 设置输入数据
auto* input_dict = request.mutable_input();
auto* input_kv = input_dict->mutable_keyvaluelist();

// 添加动作ID
base_types::Variant action_id_variant;
action_id_variant.set_type(base_types::Variant::KStringValue);
action_id_variant.set_stringvalue("move_forward");
input_kv->insert(std::make_pair("actionId", action_id_variant));

// 添加速度参数
base_types::Variant speed_variant;
speed_variant.set_type(base_types::Variant::KDoubleValue);
speed_variant.set_doublevalue(1.0);
input_kv->insert(std::make_pair("speed", speed_variant));

// 设置参数
auto* params_dict = request.mutable_params();
auto* params_kv = params_dict->mutable_keyvaluelist();

// 添加超时参数
base_types::Variant timeout_variant;
timeout_variant.set_type(base_types::Variant::KInt32Value);
timeout_variant.set_int32value(60);
params_kv->insert(std::make_pair("timeout", timeout_variant));

// 注意：ClientContext 必须在整个流生命周期内保持有效
grpc::ClientContext context;
std::unique_ptr<grpc::ClientReader<interfaces::ActionResponse>> reader;

auto status = client->Action(request, reader, context);

if (status) {
    interfaces::ActionResponse response;
    while (reader->Read(&response)) {
        // 检查返回状态
        if (response.has_ret()) {
            std::cout << "Action status: " << response.ret().code() << std::endl;
            if (!response.ret().message().empty()) {
                std::cout << "Action message: " << response.ret().message() << std::endl;
            }
        }
        
        // 处理输出数据
        if (response.has_output()) {
            const auto& output_kv = response.output().keyvaluelist();
            
            // 查找进度
            auto progress_it = output_kv.find("progress");
            if (progress_it != output_kv.end() && progress_it->second.type() == base_types::Variant::KDoubleValue) {
                std::cout << "Action progress: " << progress_it->second.doublevalue() << "%" << std::endl;
            }
            
            // 查找状态
            auto status_it = output_kv.find("status");
            if (status_it != output_kv.end() && status_it->second.type() == base_types::Variant::KStringValue) {
                std::cout << "Status: " << status_it->second.stringvalue() << std::endl;
            }
        }
    }
    
    auto final_status = reader->Finish();
    if (final_status.ok()) {
        std::cout << "Action completed successfully!" << std::endl;
    }
}
```

### 异步操作

```cpp
// 准备请求（使用新的Dictionary结构）
interfaces::SendRequest request;
auto* input_dict = request.mutable_input();
auto* input_kv = input_dict->mutable_keyvaluelist();

base_types::Variant message_variant;
message_variant.set_type(base_types::Variant::KStringValue);
message_variant.set_stringvalue("Async Hello!");
input_kv->insert(std::make_pair("content", message_variant));

// Future-based 异步调用 - 只返回操作状态
auto future = client->SendAsync(request, 5000);
// 做其他工作...
auto status = future.get();  // 只获取操作状态，不包含响应数据

if (status) {
    std::cout << "Async send completed successfully!" << std::endl;
} else {
    std::cout << "Async send failed: " << status.message() << std::endl;
}

// Callback-based 异步调用 - 推荐方式，可以获取响应数据
client->SendAsync(request, 
    [](const humanoid_robot::clientSDK::common::Status& status, const interfaces::SendResponse& response) {
        if (status) {
            std::cout << "Async send succeeded!" << std::endl;
            
            // 检查返回状态
            if (response.has_ret()) {
                std::cout << "Return code: " << response.ret().code() << std::endl;
            }
            
            // 处理输出数据
            if (response.has_output()) {
                const auto& output_kv = response.output().keyvaluelist();
                auto result_it = output_kv.find("result");
                if (result_it != output_kv.end() && result_it->second.type() == base_types::Variant::KStringValue) {
                    std::cout << "Response: " << result_it->second.stringvalue() << std::endl;
                }
            }
        } else {
            std::cout << "Async send failed: " << status.message() << std::endl;
        }
    }, 5000);
```

### 事件订阅

```cpp
interfaces::SubscribeRequest request;

// 设置输入数据
auto* input_dict = request.mutable_input();
auto* input_kv = input_dict->mutable_keyvaluelist();

// 添加主题ID
base_types::Variant topic_id_variant;
topic_id_variant.set_type(base_types::Variant::KStringValue);
topic_id_variant.set_stringvalue("robot_events");
input_kv->insert(std::make_pair("topicId", topic_id_variant));

// 设置参数
auto* params_dict = request.mutable_params();
auto* params_kv = params_dict->mutable_keyvaluelist();

// 添加超时参数（60秒订阅）
base_types::Variant timeout_variant;
timeout_variant.set_type(base_types::Variant::KInt32Value);
timeout_variant.set_int32value(60);
params_kv->insert(std::make_pair("timeout", timeout_variant));

interfaces::SubscribeResponse response;
auto status = client->Subscribe(request, response, 60000);

if (status) {
    std::cout << "Subscribed successfully!" << std::endl;
    
    // 检查返回状态
    if (response.has_ret()) {
        std::cout << "Return code: " << response.ret().code() << std::endl;
        std::cout << "Return message: " << response.ret().message() << std::endl;
    }
    
    // 查找订阅ID
    if (response.has_output()) {
        const auto& output_kv = response.output().keyvaluelist();
        auto sub_id_it = output_kv.find("subscriptionId");
        if (sub_id_it != output_kv.end() && sub_id_it->second.type() == base_types::Variant::KStringValue) {
            std::cout << "Subscription ID: " << sub_id_it->second.stringvalue() << std::endl;
        }
    }
}
```

### 使用工厂函数的简化示例

```cpp
#include "robot/client/client_callback_server.h"

using namespace humanoid_robot::clientSDK::robot;

// 使用工厂函数创建并启动回调服务器
auto callback_server = std::make_unique<ClientCallbackServer>();

// 设置消息回调
callback_server->SetSubscriptionMessageCallback(
    [](const interfaces::Notification& notification) {
        std::cout << "Received push notification!" << std::endl;
        
        // 处理推送消息
        if (notification.has_notifymessage()) {
            const auto& notify_kv = notification.notifymessage().keyvaluelist();
            
            auto message_it = notify_kv.find("message_content");
            if (message_it != notify_kv.end() && message_it->second.type() == base_types::Variant::KStringValue) {
                std::cout << "Push Message: " << message_it->second.stringvalue() << std::endl;
            }
            
            auto event_it = notify_kv.find("event_type");
            if (event_it != notify_kv.end() && event_it->second.type() == base_types::Variant::KStringValue) {
                std::cout << "Event Type: " << event_it->second.stringvalue() << std::endl;
            }
        }
    });

// 启动回调服务器（自动分配端口）
auto server_status = callback_server->Start("0.0.0.0", 50052);
if (!server_status) {
    std::cout << "Failed to start callback server: " << server_status.message() << std::endl;
    return -1;
}

std::cout << "Callback server started at: " << callback_server->GetClientEndpoint() << std::endl;

### 完整的订阅推送流程示例
```

```cpp
#include "robot/client/interfaces_client.h"
#include "robot/client/client_callback_server.h"

using namespace humanoid_robot::clientSDK::robot;

// 第一步：创建并启动回调服务器
auto callback_server = std::make_unique<ClientCallbackServer>();

// 设置消息回调
callback_server->SetSubscriptionMessageCallback(
    [](const interfaces::Notification& notification) {
        std::cout << "Received push notification!" << std::endl;
        
        // 处理推送消息
        if (notification.has_notifymessage()) {
            const auto& notify_kv = notification.notifymessage().keyvaluelist();
            
            auto message_it = notify_kv.find("message");
            if (message_it != notify_kv.end() && message_it->second.type() == base_types::Variant::KStringValue) {
                std::cout << "Push Message: " << message_it->second.stringvalue() << std::endl;
            }
        }
    });

// 启动回调服务器（自动分配端口）
int assigned_port;
auto server_status = callback_server->StartWithAutoPort("0.0.0.0", assigned_port);
if (!server_status) {
    std::cout << "Failed to start callback server: " << server_status.message() << std::endl;
    return -1;
}

std::cout << "Callback server started on port: " << assigned_port << std::endl;

// 第二步：创建主客户端连接到 SDK 服务器
std::unique_ptr<InterfacesClient> client;
auto client_status = CreateInterfacesClientLegacy("localhost", 50051, client);
if (!client_status) {
    std::cout << "Failed to connect to SDK server: " << client_status.message() << std::endl;
    return -1;
}

// 第三步：发送订阅请求，包含回调服务器的监听地址
interfaces::SubscribeRequest request;

// 设置输入数据
auto* input_dict = request.mutable_input();
auto* input_kv = input_dict->mutable_keyvaluelist();

// 添加主题ID
base_types::Variant topic_id_variant;
topic_id_variant.set_type(base_types::Variant::KStringValue);
topic_id_variant.set_stringvalue("robot_events");
input_kv->insert(std::make_pair("topicId", topic_id_variant));

// 关键：添加回调服务器的监听地址
base_types::Variant callback_endpoint_variant;
callback_endpoint_variant.set_type(base_types::Variant::KStringValue);
callback_endpoint_variant.set_stringvalue(callback_server->GetClientEndpoint()); // "0.0.0.0:port"
input_kv->insert(std::make_pair("client_endpoint", callback_endpoint_variant));

// 设置参数
auto* params_dict = request.mutable_params();
auto* params_kv = params_dict->mutable_keyvaluelist();

// 添加超时参数
base_types::Variant timeout_variant;
timeout_variant.set_type(base_types::Variant::KInt32Value);
timeout_variant.set_int32value(300); // 5分钟订阅
params_kv->insert(std::make_pair("timeout", timeout_variant));

// 发送订阅请求
interfaces::SubscribeResponse response;
auto subscribe_status = client->Subscribe(request, response, 60000);

if (subscribe_status) {
    std::cout << "Subscription successful!" << std::endl;
    
    // 检查返回状态
    if (response.has_ret()) {
        std::cout << "Return code: " << response.ret().code() << std::endl;
    }
    
    // 获取订阅ID
    if (response.has_output()) {
        const auto& output_kv = response.output().keyvaluelist();
        auto sub_id_it = output_kv.find("subscriptionId");
        if (sub_id_it != output_kv.end() && sub_id_it->second.type() == base_types::Variant::KStringValue) {
            std::cout << "Subscription ID: " << sub_id_it->second.stringvalue() << std::endl;
        }
    }
    
    // 第四步：此时 SDK 服务器会根据 callbackEndpoint 创建客户端连接到我们的回调服务器
    // 当有新消息时，SDK 服务器会主动推送到回调服务器
    
    std::cout << "Waiting for push messages... (Press Enter to stop)" << std::endl;
    std::cin.get();
    
} else {
    std::cout << "Subscription failed: " << subscribe_status.message() << std::endl;
}

// 清理资源
callback_server->Stop();
client->Disconnect();
```

## API 参考

### 主要类

#### `InterfacesClient`

核心客户端类，提供所有 InterfaceService 操作。

**连接方法:**
- `Connect(server_address, port)` - 连接到服务器
- `Connect(target)` - 使用目标字符串连接
- `Disconnect()` - 断开连接
- `IsConnected()` - 检查连接状态

**同步方法:**
- `Send(request, response, timeout_ms)` - 发送消息
- `Query(request, response, timeout_ms)` - 查询资源
- `Action(request, reader, context)` - 执行动作（流式响应）
- `Subscribe(request, response, timeout_ms)` - 订阅事件
- `Unsubscribe(request, response, timeout_ms)` - 取消订阅

**异步方法:**
- `SendAsync(request, timeout_ms)` - 返回 Future（仅状态）
- `SendAsync(request, callback, timeout_ms)` - 使用回调（推荐，包含响应数据）
- `QueryAsync(request, timeout_ms)` - 返回 Future（仅状态）
- `QueryAsync(request, callback, timeout_ms)` - 使用回调（推荐，包含响应数据）

**重要说明:** Future-based 异步方法只返回操作状态，不包含响应数据。如需获取响应数据，请使用 Callback-based 异步方法。

**工具方法:**
- `GetChannelState()` - 获取通道状态
- `WaitForChannelReady(timeout_ms)` - 等待通道就绪

#### `ClientCallbackServer`

客户端回调服务器，用于接收来自服务器的主动推送消息。

**生命周期管理:**
- `Start(listen_address, port)` - 启动服务器
- `StartWithAutoPort(listen_address, assigned_port)` - 启动服务器（自动分配端口）
- `Stop()` - 停止服务器
- `IsRunning()` - 检查运行状态
- `GetListenAddress()` - 获取监听地址
- `GetListenPort()` - 获取监听端口

**回调函数注册:**
- `SetSubscriptionMessageCallback(callback)` - 注册消息回调（接收通知消息）

**回调函数类型:**
```cpp
using SubscriptionMessageCallback = std::function<void(const interfaces::Notification &)>;
```

**注意:** 
- 消息结构已更新为 Dictionary 格式，回调函数现在接收 `interfaces::Notification` 消息
- **重要**: 必须在启动服务器之前设置回调函数，服务器运行时不能修改回调函数

### 工厂函数

```cpp
// 创建并启动客户端回调服务器的便捷函数
std::pair<std::unique_ptr<ClientCallbackServer>, Status> CreateCallbackServer(
    const std::string& listen_address,
    int port = 0,                                    // 0表示自动分配端口
    SubscriptionMessageCallback message_callback = nullptr
);
```

### 便捷函数

```cpp
// Legacy 便捷函数（返回 unique_ptr）
humanoid_robot::clientSDK::common::Status CreateInterfacesClientLegacy(
    const std::string& server_address,
    int port,
    std::unique_ptr<InterfacesClient>& client
);

// 工厂函数（返回 shared_ptr，推荐用于异步操作）
humanoid_robot::clientSDK::factory::CreateInterfacesClient(
    const std::string& server_address,
    int port,
    std::shared_ptr<robot::InterfacesClient>& client
);

humanoid_robot::clientSDK::factory::CreateInterfacesClient(
    const std::string& target,
    std::shared_ptr<robot::InterfacesClient>& client
);
```

### 重要变更说明

#### Action 方法签名变更

`Action` 方法现在要求调用者管理 `grpc::ClientContext` 的生命周期：

```cpp
// 新签名
Status Action(
    const interfaces::ActionRequest &request,
    std::unique_ptr<::grpc::ClientReader<::interfaces::ActionResponse>> &reader,
    grpc::ClientContext &context);

// 使用示例
grpc::ClientContext context;  // 必须在流生命周期内保持有效
std::unique_ptr<grpc::ClientReader<interfaces::ActionResponse>> reader;
auto status = client->Action(request, reader, context);
```

**重要提示：** `context` 参数必须在整个流操作期间保持有效。不要在栈上创建 context 然后立即销毁它。

## 错误处理

所有操作返回 `humanoid_robot::common::Status` 对象：

```cpp
auto status = client->Send(request, response);

if (status) {
    // 操作成功
    std::cout << "Success!" << std::endl;
} else {
    // 操作失败
    std::cout << "Error: " << status.message() << std::endl;
    std::cout << "Code: " << status.code().value() << std::endl;
}
```

常见错误码：
- `std::errc::not_connected` - 客户端未连接
- `std::errc::timed_out` - 操作超时
- `std::errc::connection_refused` - 连接被拒绝
- `std::errc::host_unreachable` - 服务不可用

## 超时配置

所有操作都支持超时设置：

```cpp
// 短超时用于快速操作
client->Send(request, response, 3000);  // 3秒

// 长超时用于复杂操作
client->Query(request, response, 30000); // 30秒

// 流式订阅可以设置0表示无超时
client->Subscribe(request, response, 0);  // 无超时
```

## 最佳实践

1. **连接管理**: 在应用程序开始时创建客户端连接，在结束时断开
2. **错误处理**: 始终检查操作返回的状态
3. **超时设置**: 根据操作复杂度设置合理的超时时间
4. **异步操作选择**: 
   - 如果只需要知道操作是否成功，使用 Future-based 异步方法
   - 如果需要获取响应数据，使用 Callback-based 异步方法
   - 对于耗时操作使用异步方法避免阻塞
5. **客户端类型选择**:
   - 同步操作和简单异步：使用 `unique_ptr<InterfacesClient>`
   - 复杂异步操作：使用 `shared_ptr<InterfacesClient>`（通过工厂函数创建）
6. **资源清理**: 确保正确断开连接和释放资源
7. **流式操作**: 
   - 对于 `Action` 方法，确保 `ClientContext` 在整个流生命周期内有效
   - 正确处理流的结束和错误情况
   - 使用适当的超时设置
8. **多线程回调架构最佳实践**: 
   - **线程安全**: 回调函数中的处理逻辑应该是线程安全的
   - **参数兼容性**: 使用 `client_endpoint` 参数名（支持 `callbackurl` 向后兼容）
   - **必须先设置回调函数，再启动服务器**（服务器运行时不能修改回调）
   - 先启动回调服务器，再发送订阅请求
   - 在订阅请求中正确设置 `client_endpoint` 字段
   - 确保回调服务器的监听地址对 SDK 服务器可达
   - 使用 `StartWithAutoPort()` 避免端口冲突
   - 使用 `CreateCallbackServer()` 工厂函数简化创建过程
   - 在应用程序退出前正确停止回调服务器
   - **非阻塞处理**: 回调函数应快速返回，避免长时间阻塞
   - **错误隔离**: 单个回调失败不应影响其他回调的执行
   - **资源管理**: 确保回调中使用的资源得到正确管理

## 示例程序

运行示例程序：

```bash
cd /path/to/build
./bin/linux_x64/release/interfaces_client_example
```

示例程序演示了：
- 连接管理
- 同步操作
- 异步操作  
- 流式操作
- 多线程回调订阅
- 错误处理

## 性能特性

### 多线程回调性能
- **非阻塞设计**: 服务器回调在独立线程中执行，不影响主服务性能
- **并发处理**: 支持多个订阅并发接收回调消息
- **线程安全**: 所有订阅操作都有互斥锁保护
- **自动清理**: 心跳监控每30秒检查一次，自动清理失效订阅

### 连接管理
- **持久连接**: 订阅建立后保持长期连接
- **自动重连**: 支持网络中断后的自动重连（客户端负责）
- **参数兼容**: 同时支持 `client_endpoint` 和 `callbackurl` 参数
- **端口管理**: 支持自动端口分配和手动端口指定

## 依赖要求

- C++17 或更高版本
- gRPC 1.71.0+
- Protocol Buffers 5.29.3+
- 预编译的 interfaces protobuf 库
- 预编译的 common protobuf 库

## 构建依赖

本模块依赖以下组件：
- `PB::common` - 通用 protobuf 定义
- `PB::interfaces` - 接口 protobuf 定义
- `gRPC::grpc++` - gRPC C++ 库
- `protobuf::libprotobuf` - Protocol Buffers 库

Client-SDK 包含两个主要组件：
1. **InterfacesClient** - 主动发起请求的客户端
2. **ClientCallbackServer** - 接收服务器主动推送的回调服务器

这种双向通信设计允许：
- 客户端主动请求服务（通过 InterfacesClient）
- 服务器主动推送消息给客户端（通过 ClientCallbackServer）
- 多线程非阻塞回调处理（服务器端使用 detached 线程）
- 线程安全的订阅管理和心跳监控

## 多线程架构优势

- **高性能**: 非阻塞的回调处理，支持大量并发订阅
- **可靠性**: 自动心跳监控和失效订阅清理
- **可扩展性**: 线程安全设计，适用于大规模部署
- **兼容性**: 支持多种参数名，向后兼容旧版本客户端

## 订阅推送架构说明

### 多线程回调架构

**架构特点：**
- **非阻塞设计** - 服务器使用独立线程处理回调，避免阻塞主服务
- **线程安全** - 订阅管理使用互斥锁保护，支持并发访问
- **持久化连接** - 服务器维护客户端连接，支持长期订阅
- **自动清理** - 心跳监控自动清理失效订阅
- **参数兼容** - 支持 `client_endpoint` 和 `callbackurl` 参数名

### 工作原理

1. **客户端启动回调服务器**：
   - 客户端创建并启动一个 gRPC 服务器（ClientCallbackServer）
   - 该服务器监听特定端口，等待接收推送消息

2. **发送订阅请求**：
   - 客户端通过 InterfacesClient 向 SDK 服务器发送 Subscribe 请求
   - 请求中包含 `client_endpoint` 字段，告知 SDK 服务器回调服务器的监听地址

3. **SDK 服务器建立反向连接**：
   - SDK 服务器收到订阅请求后，根据 `client_endpoint` 创建客户端连接
   - SDK 服务器在独立线程中连接到客户端的回调服务器

4. **多线程主动推送消息**：
   - 当有新的事件或消息时，SDK 服务器在独立的 detached 线程中主动调用客户端回调服务器的接口
   - 客户端通过注册的回调函数接收并处理推送消息
   - 多个回调可以并发执行，不会相互阻塞

### 多线程架构图
```
┌─────────────────┐    Subscribe请求     ┌─────────────────┐
│                 │ ─────────────────→   │                 │
│  Client SDK     │ (含client_endpoint)  │  SDK Server     │
│                 │                      │  [多线程架构]   │
│ ┌─────────────┐ │                      │ ┌─────────────┐ │
│ │InterfacesClient│                      │ │InterfaceService│ │
│ └─────────────┘ │                      │ │[主线程]     │ │
│                 │                      │ └─────────────┘ │
│ ┌─────────────┐ │   推送消息            │ ┌─────────────┐ │
│ │CallbackServer│ │ ←─────────────────  │ │CallbackClient│ │
│ │[线程安全]   │ │  [detached线程]      │ │[回调线程池] │ │
│ └─────────────┘ │                      │ └─────────────┘ │
└─────────────────┘                      └─────────────────┘
                                           ┌─────────────┐
                                           │订阅管理     │
                                           │[互斥锁保护] │
                                           │心跳监控     │
                                           └─────────────┘
```

## 许可证

版权所有 (c) 2025 Humanoid Robot, Inc. 保留所有权利。

---

*最后更新: 2025年8月7日*
*版本: 多线程回调架构版本*
