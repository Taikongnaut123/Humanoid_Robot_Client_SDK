# InterfacesClient - gRPC客户端库

## 概述

InterfacesClient 是基于 gRPC 的 C++ 客户端库，为 InterfaceService 提供完整的同步和异步接口。它实现了 `interfaces_grpc.proto` 中定义的所有操作。

## 目录结构

```
Client-SDK/
├── include/robot/client/
│   └── interfaces_client.h      # 客户端头文件
├── source/robot/client/
│   └── interfaces_client.cpp    # 客户端实现
└── example/
    └── interfaces_client_example.cpp  # 使用示例
```

## 构建和链接

### CMake 配置

客户端库会自动与以下库链接：
- `libinterfaces` - interfaces protobuf 库
- `libcommon` - common protobuf 库 
- `protobuf::libprotobuf` - Google Protocol Buffers
- `gRPC::grpc++` - gRPC C++ 库

### 生成的文件

编译后会生成：
- `librobot_client.so` - 客户端动态库
- `interfaces_client_example` - 示例可执行文件

## 功能特性

### 连接管理
- 自动连接和断开连接
- 连接状态检查
- 连接超时处理
- 服务器信息获取

### 同步操作
- **Create** - 创建资源
- **Send** - 发送消息  
- **Delete** - 删除资源
- **Query** - 查询资源
- **BatchCreate** - 批量创建
- **HealthCheck** - 健康检查
- **Unsubscribe** - 取消订阅

### 异步操作
- Future-based 异步调用
- Callback-based 异步调用
- 非阻塞操作
- 错误处理

### 流式操作
- **Subscribe** - 事件订阅 (服务器流)
- 实时事件处理
- 流错误处理
- 超时控制

## 使用示例

### 基本连接

```cpp
#include "robot/client/interfaces_client.h"

using namespace humanoid_robot::robot;

// 方法1: 使用便捷函数
std::unique_ptr<InterfacesClient> client;
auto status = CreateInterfacesClient("localhost", 50051, client);

// 方法2: 手动创建和连接
auto client = std::make_unique<InterfacesClient>();
auto status = client->Connect("localhost", 50051);
```

### 健康检查

```cpp
interfaces::HealthCheckRequest request;
request.set_service("InterfaceService");

interfaces::HealthCheckResponse response;
auto status = client->HealthCheck(request, response, 3000);

if (status) {
    std::cout << "Service status: " << static_cast<int>(response.status()) << std::endl;
    std::cout << "Message: " << response.message() << std::endl;
}
```

### 创建资源

```cpp
interfaces::CreateRequest request;

// 设置请求数据
auto* req_data = request.mutable_requestdata();
auto* req_items = req_data->mutable_keyvaluelist();

// 添加名称字段
base_types::Variant name_variant;
name_variant.set_type(base_types::Variant::KStringValue);
name_variant.set_stringvalue("my_resource");
(*req_items)["name"] = name_variant;

// 设置参数
auto* params = request.mutable_params();
params->set_timeout(30);
params->set_correlationid("req-001");

interfaces::CreateResponse response;
auto status = client->Create(request, response, 5000);

if (status) {
    std::cout << "Created resource ID: " << response.resourceid() << std::endl;
}
```

### 异步操作

```cpp
// Future-based 异步调用
auto future = client->CreateAsync(request, 5000);
// 做其他工作...
auto status = future.get();

// Callback-based 异步调用
client->CreateAsync(request, 
    [](const Status& status, const interfaces::CreateResponse& response) {
        if (status) {
            std::cout << "Async create succeeded: " << response.resourceid() << std::endl;
        }
    }, 5000);
```

### 事件订阅

```cpp
interfaces::SubscribeRequest request;
request.set_topicid("events");

auto* params = request.mutable_params();
params->set_timeout(60); // 60秒订阅

auto status = client->Subscribe(request,
    [](const interfaces::SubscribeResponse& response) {
        std::cout << "Received event: " << response.message() << std::endl;
    }, 60000);
```

### 查询操作

```cpp
interfaces::QueryRequest request;
request.set_queryid("find_all");
request.set_limit(10);
request.set_offset(0);

interfaces::QueryResponse response;
auto status = client->Query(request, response, 5000);

if (status) {
    std::cout << "Found " << response.totalcount() << " total results" << std::endl;
    std::cout << "Returned " << response.results_size() << " items" << std::endl;
}
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
- `Create(request, response, timeout_ms)` - 创建资源
- `Send(request, response, timeout_ms)` - 发送消息
- `Delete(request, response, timeout_ms)` - 删除资源
- `Query(request, response, timeout_ms)` - 查询资源
- `BatchCreate(request, response, timeout_ms)` - 批量创建
- `HealthCheck(request, response, timeout_ms)` - 健康检查
- `Unsubscribe(request, response, timeout_ms)` - 取消订阅

**异步方法:**
- `CreateAsync(request, timeout_ms)` - 返回 Future
- `CreateAsync(request, callback, timeout_ms)` - 使用回调

**流式方法:**
- `Subscribe(request, callback, timeout_ms)` - 事件订阅

**工具方法:**
- `GetChannelState()` - 获取通道状态
- `WaitForChannelReady(timeout_ms)` - 等待通道就绪
- `GetServerInfo(server_info)` - 获取服务器信息

### 便捷函数

```cpp
// 创建并连接客户端
humanoid_robot::common::Status CreateInterfacesClient(
    const std::string& server_address,
    int port,
    std::unique_ptr<InterfacesClient>& client
);

humanoid_robot::common::Status CreateInterfacesClient(
    const std::string& target,
    std::unique_ptr<InterfacesClient>& client
);
```

## 错误处理

所有操作返回 `humanoid_robot::common::Status` 对象：

```cpp
auto status = client->HealthCheck(request, response);

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
client->HealthCheck(request, response, 3000);  // 3秒

// 长超时用于复杂操作
client->BatchCreate(request, response, 30000); // 30秒

// 流式订阅可以设置0表示无超时
client->Subscribe(request, callback, 0);  // 无超时
```

## 最佳实践

1. **连接管理**: 在应用程序开始时创建客户端连接，在结束时断开
2. **错误处理**: 始终检查操作返回的状态
3. **超时设置**: 根据操作复杂度设置合理的超时时间
4. **异步操作**: 对于耗时操作使用异步方法避免阻塞
5. **资源清理**: 确保正确断开连接和释放资源

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
- 流式订阅
- 错误处理

## 依赖要求

- C++17 或更高版本
- gRPC 1.71.0+
- Protocol Buffers 5.29.3+
- 预编译的 interfaces protobuf 库
- 预编译的 common protobuf 库

## 许可证

版权所有 (c) 2025 Humanoid Robot, Inc. 保留所有权利。
