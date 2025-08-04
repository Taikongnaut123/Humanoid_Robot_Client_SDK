# 人形机器人接口服务需求说明文档

## 1. 项目概述

### 1.1 项目背景

人形机器人接口服务（Humanoid Robot Interface Service）是一个基于 gRPC 的统一接口服务系统，为人形机器人的各个功能模块提供标准化的通信接口。

### 1.2 系统架构

- **通信协议**: gRPC + Protocol Buffers
- **架构模式**: Client-Server 架构，支持持久订阅
- **消息格式**: 通用键值对结构，灵活扩展
- **部署方式**: 微服务架构，各模块独立部署
- **编程语言**: C++17

### 1.3 设计目标

- 提供统一的机器人功能接口
- 支持实时数据传输和事件订阅
- 使用灵活的键值对消息格式
- 确保系统的可扩展性和模块化
- 保证接口的类型安全和版本兼容性

## 2. 核心消息结构

### 2.1 接口消息结构

所有接口都使用相同的消息结构，只是在消息类型名称上加前缀：

```protobuf
// 创建操作
message CreateRequest {
  base_types.Dictionary input = 1;       // 输入数据（键值对）
  base_types.Dictionary params = 2;      // 参数数据（键值对）
}
message CreateResponse {
  base_types.Dictionary output = 1;      // 输出数据（键值对）
  int32 ret = 2;                        // 返回码（0=成功，非0=错误码）
}

// 发送操作
message SendRequest {
  base_types.Dictionary input = 1;       // 输入数据（键值对）
  base_types.Dictionary params = 2;      // 参数数据（键值对）
}
message SendResponse {
  base_types.Dictionary output = 1;      // 输出数据（键值对）
  int32 ret = 2;                        // 返回码（0=成功，非0=错误码）
}

// 删除操作
message DeleteRequest {
  base_types.Dictionary input = 1;       // 输入数据（键值对）
  base_types.Dictionary params = 2;      // 参数数据（键值对）
}
message DeleteResponse {
  base_types.Dictionary output = 1;      // 输出数据（键值对）
  int32 ret = 2;                        // 返回码（0=成功，非0=错误码）
}

// 查询操作
message QueryRequest {
  base_types.Dictionary input = 1;       // 输入数据（键值对）
  base_types.Dictionary params = 2;      // 参数数据（键值对）
}
message QueryResponse {
  base_types.Dictionary output = 1;      // 输出数据（键值对）
  int32 ret = 2;                        // 返回码（0=成功，非0=错误码）
}
```

### 2.2 操作类型枚举

```protobuf
// 操作类型枚举 - 定义所有支持的操作
enum ActionType {
  ACTION_UNKNOWN = 0;
  
  // 导航模块操作 (100-199)
  NAVIGATION_SET_GOAL = 100;           // 设置导航目标
  NAVIGATION_CANCEL = 101;             // 取消导航任务
  NAVIGATION_GET_GOAL_POSE = 102;      // 获取目标位姿
  NAVIGATION_GET_3D_MAP = 103;         // 获取3D地图
  NAVIGATION_GET_GLOBAL_COSTMAP = 104; // 获取全局代价地图
  NAVIGATION_GET_LOCAL_COSTMAP = 105;  // 获取局部代价地图
  NAVIGATION_GET_PLANNED_PATH = 106;   // 获取规划路径
  
  // 感知模块操作 (200-299)
  PERCEPTION_PROCESS_IMAGE = 200;      // 处理图像数据
  PERCEPTION_GET_RESULTS = 201;        // 获取感知结果
  PERCEPTION_GET_DETECTION = 202;      // 获取检测结果
  PERCEPTION_GET_SEGMENTATION = 203;   // 获取分割结果
  
  // 决策模块操作 (300-399)
  DECISION_SEND_TASK = 300;            // 发送任务指令
  DECISION_GET_TASK_PLANNING = 301;    // 获取任务规划
  DECISION_GET_FACT_MEMORY = 302;      // 获取事实记忆库
  
  // 控制模块操作 (400-499)
  CONTROL_EXECUTE_ARM_ACTION = 400;    // 执行机械臂动作
  CONTROL_EXECUTE_BASE_ACTION = 401;   // 执行底盘动作
  CONTROL_SET_JOINT_POSITION = 402;    // 设置关节位置
  
  // 传感器模块操作 (500-599)
  SENSOR_GET_CAMERA_DATA = 500;        // 获取相机数据
  SENSOR_GET_IMU_DATA = 501;           // 获取IMU数据
  SENSOR_GET_TF_TRANSFORMS = 502;      // 获取TF变换
}
```

### 2.3 返回码定义

- **0**: 成功
- **-1**: 通用错误
- **-2**: 参数错误
- **-3**: 超时错误
- **-4**: 网络错误
- **-5**: 权限错误
- **-100～-199**: 导航模块错误
- **-200～-299**: 感知模块错误
- **-300～-399**: 决策模块错误
- **-400～-499**: 控制模块错误
- **-500～-599**: 传感器模块错误

## 3. 接口服务定义

### 3.1 主接口服务

```protobuf
service InterfaceService {
  // 创建资源接口
  rpc Create(CreateRequest) returns (CreateResponse);

  // 发送数据接口
  rpc Send(SendRequest) returns (SendResponse);

  // 删除资源接口
  rpc Delete(DeleteRequest) returns (DeleteResponse);

  // 查询数据接口
  rpc Query(QueryRequest) returns (QueryResponse);

  // 订阅相关接口
  rpc Subscribe(SubscribeRequest) returns (stream SubscribeResponse);
  rpc Unsubscribe(UnsubscribeRequest) returns (UnsubscribeResponse);
}
```

### 3.2 消息结构说明

所有接口都使用统一的键值对结构：

- **请求消息**: 包含 `input`（输入数据）和 `params`（参数数据）两个 Dictionary 字段
- **响应消息**: 包含 `output`（输出数据）Dictionary 字段和 `ret`（返回码）整型字段
- **消息命名**: 通过在消息类型前加接口名前缀来区分不同操作（如 CreateRequest、SendRequest 等）
- **操作标识**: 所有请求的 `input` 字段中都包含 `action` 键，其值为 `ActionType` 枚举类型，用于指定具体的操作类型

所有业务逻辑通过 `input` 和 `params` 中的键值对来实现，提供最大的灵活性和扩展性。

### 3.3 使用示例

```protobuf
// 示例：导航设置目标的请求
SendRequest {
  input: {
    "action": NAVIGATION_SET_GOAL,        // 操作类型（枚举）
    "target_pose": {...},                 // 具体业务数据
    "frame_id": "map",
    "task_id": "nav_task_001"
  },
  params: {
    "tolerance_position": 0.1,            // 业务参数
    "tolerance_orientation": 0.1,
    "timeout": 30
  }
}
```

## 4. 功能模块接口定义

### 4.1 导航模块 (Navigation Module)

#### 4.1.1 发送导航目标点

**接口调用**: `Send(SendRequest)`
**功能标识**: `input.action = NAVIGATION_SET_GOAL`

**输入参数 (SendRequest.input)**:

```yaml
action: NAVIGATION_SET_GOAL       # 操作类型枚举
target_pose:
  position:
    x: 1.0                    # 目标位置X (米)
    y: 2.0                    # 目标位置Y (米)
    z: 0.0                    # 目标位置Z (米)
  orientation:
    x: 0.0                    # 四元数X
    y: 0.0                    # 四元数Y
    z: 0.0                    # 四元数Z
    w: 1.0                    # 四元数W
frame_id: "map"               # 坐标系ID
task_id: "nav_task_001"       # 任务唯一标识
```

**参数 (SendRequest.params)**:

```yaml
tolerance_position: 0.1       # 位置容差 (米)
tolerance_orientation: 0.1    # 角度容差 (弧度)
timeout: 30                   # 超时时间 (秒)
```

**输出结果 (SendResponse.output)**:

```yaml
#### 4.1.2 结束导航任务

**接口调用**: `Delete(DeleteRequest)`
**功能标识**: `input.action = NAVIGATION_CANCEL`

**输入参数 (DeleteRequest.input)**:

```yaml
action: NAVIGATION_CANCEL         # 操作类型枚举
task_id: "nav_task_001"       # 要取消的任务ID (空则取消所有)
```

**参数 (DeleteRequest.params)**:

```yaml
force_stop: true              # 是否强制停止
```

#### 4.1.3 获取导航目标点位姿

**接口调用**: `Query(QueryRequest)`
**功能标识**: `input.action = NAVIGATION_GET_GOAL_POSE`

**输出结果 (QueryResponse.output)**:

```yaml
goal_pose:                    # 当前目标位姿
  position: {x: 1.0, y: 2.0, z: 0.0}
  orientation: {x: 0.0, y: 0.0, z: 0.0, w: 1.0}
frame_id: "map"               # 坐标系
task_id: "nav_task_001"       # 关联任务ID
navigation_status: "executing" # 导航状态
```

#### 4.1.4 获取3D地图

**接口调用**: `Query(QueryRequest)`
**功能标识**: `input.action = NAVIGATION_GET_3D_MAP`

**输出结果 (QueryResponse.output)**:

```yaml
point_cloud:                  # 点云数据
  header:
    timestamp: 1672531200000  # 时间戳
    frame_id: "map"           # 坐标系
  points:                     # 点云数组
    - {x: 1.0, y: 2.0, z: 0.5}
    - {x: 1.1, y: 2.1, z: 0.6}
metadata:
  resolution: 0.05            # 分辨率
  width: 1000                 # 宽度
  height: 1000                # 高度
```

#### 4.1.5 获取全局代价地图

**接口调用**: `Query(QueryRequest)`
**功能标识**: `input.action = NAVIGATION_GET_GLOBAL_COSTMAP`

#### 4.1.6 获取局部代价地图

**接口调用**: `Query(QueryRequest)`
**功能标识**: `input.action = NAVIGATION_GET_LOCAL_COSTMAP`

#### 4.1.7 获取路径规划结果

**接口调用**: `Query(QueryRequest)`
**功能标识**: `input.action = NAVIGATION_GET_PLANNED_PATH`

**输出结果 (QueryResponse.output)**:

```yaml
path:                         # 规划路径点序列
  - pose:
      position: {x: 0.0, y: 0.0, z: 0.0}
      orientation: {x: 0.0, y: 0.0, z: 0.0, w: 1.0}
    timestamp: 1672531200000
  - pose:
      position: {x: 0.5, y: 0.5, z: 0.0}
      orientation: {x: 0.0, y: 0.0, z: 0.1, w: 0.99}
    timestamp: 1672531201000
total_distance: 5.2           # 路径总长度 (米)
estimated_time: 10.5          # 预估时间 (秒)
algorithm_used: "A*"          # 使用的算法
```

### 4.2 感知模块 (Perception Module)

#### 4.2.1 发送图像数据

**接口调用**: `Send(SendRequest)`
**功能标识**: `input.action = PERCEPTION_PROCESS_IMAGE`

**输入参数 (SendRequest.input)**:

```yaml
action: PERCEPTION_PROCESS_IMAGE  # 操作类型枚举
image:
  header:
    timestamp: 1672531200000
    frame_id: "camera_link"
  encoding: "rgb8"            # 图像编码格式
  width: 640                  # 图像宽度
  height: 480                 # 图像高度
  data: "base64_encoded_data" # Base64编码的图像数据
camera_info:
  k: [525.0, 0.0, 320.0, 0.0, 525.0, 240.0, 0.0, 0.0, 1.0] # 内参矩阵
  d: [0.1, -0.2, 0.0, 0.0, 0.1] # 畸变参数
source_id: "front_camera"     # 图像源标识
```

**参数 (SendRequest.params)**:

```yaml
processing_types: ["detection", "segmentation"] # 处理类型
confidence_threshold: 0.5     # 置信度阈值
```

#### 4.2.2 获取感知整体结果

**接口调用**: `Query(QueryRequest)`
**功能标识**: `input.action = PERCEPTION_GET_RESULTS`

**输出结果 (QueryResponse.output)**:

```yaml
detections:                   # 检测结果
  - label: "person"
    confidence: 0.95
    bbox: {x: 100, y: 100, width: 50, height: 100}
    position_3d: {x: 2.0, y: 0.5, z: 1.7}
segmentation:                 # 分割结果
  mask_image:
    encoding: "mono8"
    width: 640
    height: 480
    data: "base64_encoded_mask"
  segments:
    - category: "floor"
      area: 15000
      centroid: {x: 320, y: 400}
tracking:                     # 跟踪结果
  - object_id: "person_001"
    label: "person"
    trajectory:
      - {x: 2.0, y: 0.5, timestamp: 1672531200000}
      - {x: 2.1, y: 0.6, timestamp: 1672531201000}
timestamp: 1672531200000
```

#### 4.2.3 获取检测结果

**接口调用**: `Query(QueryRequest)`
**功能标识**: `input.action = PERCEPTION_GET_DETECTION`

#### 4.2.4 获取分割结果

**接口调用**: `Query(QueryRequest)`
**功能标识**: `input.action = PERCEPTION_GET_SEGMENTATION`

### 4.3 决策模块 (Decision Module)

#### 4.3.1 发送任务指令

**接口调用**: `Send(SendRequest)`
**功能标识**: `input.action = DECISION_SEND_TASK`

**输入参数 (SendRequest.input)**:

```yaml
action: DECISION_SEND_TASK        # 操作类型枚举
task_type: "navigation_and_pick" # 任务类型
parameters:
  target_object: "cup"        # 目标物体
  pickup_location: {x: 1.0, y: 2.0, z: 0.8}
  drop_location: {x: 3.0, y: 1.0, z: 0.8}
priority: 1                   # 优先级 (1=高, 2=中, 3=低)
parent_task_id: ""           # 父任务ID
```

#### 4.3.2 获取任务规划结果

**接口调用**: `Query(QueryRequest)`
**功能标识**: `input.action = DECISION_GET_TASK_PLANNING`

**输出结果 (QueryResponse.output)**:

```yaml
steps:                        # 任务步骤序列
  - step_id: 1
    action: "navigate_to"
    parameters:
      target: {x: 1.0, y: 2.0, z: 0.0}
    estimated_duration: 10.0
  - step_id: 2
    action: "pick_object"
    parameters:
      object_id: "cup_001"
    estimated_duration: 5.0
total_estimated_duration: 15.0 # 预估总时间
required_resources: ["arm", "base", "perception"] # 所需资源
task_status: "planned"        # 任务状态
```

#### 4.3.3 获取事实记忆库信息

**接口调用**: `Query(QueryRequest)`
**功能标识**: `input.action = DECISION_GET_FACT_MEMORY`

**输出结果 (QueryResponse.output)**:

```yaml
facts:                        # 事实条目
  - fact_id: "fact_001"
    category: "object_location"
    content:
      object: "cup"
      location: {x: 1.0, y: 2.0, z: 0.8}
      confidence: 0.9
    timestamp: 1672531200000
categories: ["object_location", "task_history", "environmental_state"]
last_updated: 1672531200000
total_count: 150
```

### 4.4 控制模块 (Control Module)

#### 4.4.1 执行机械臂动作

**接口调用**: `Send(SendRequest)`
**功能标识**: `input.action = CONTROL_EXECUTE_ARM_ACTION`

**输入参数 (SendRequest.input)**:

```yaml
action: CONTROL_EXECUTE_ARM_ACTION # 操作类型枚举
target_pose:
  position: {x: 0.5, y: 0.3, z: 0.8}
  orientation: {x: 0.0, y: 0.707, z: 0.0, w: 0.707}
action_type: "pick"           # 动作类型: pick/place/move/home
control_mode: "position"      # 控制模式: position/velocity/force
```

**参数 (SendRequest.params)**:

```yaml
velocity: 0.1                 # 运动速度 (m/s)
acceleration: 0.05            # 加速度 (m/s²)
force_limit: 10.0             # 力限制 (N)
```

### 4.5 传感器数据模块 (Sensor Data Module)

#### 4.5.1 获取相机数据

**接口调用**: `Query(QueryRequest)`
**功能标识**: `input.action = SENSOR_GET_CAMERA_DATA`

**参数 (QueryRequest.params)**:

```yaml
camera_id: "front_camera"     # 相机标识
```

**输出结果 (QueryResponse.output)**:

```yaml
image:
  header:
    timestamp: 1672531200000
    frame_id: "camera_link"
  encoding: "rgb8"
  width: 640
  height: 480
  data: "base64_encoded_data"
camera_info:
  k: [525.0, 0.0, 320.0, 0.0, 525.0, 240.0, 0.0, 0.0, 1.0]
  d: [0.1, -0.2, 0.0, 0.0, 0.1]
camera_id: "front_camera"
timestamp: 1672531200000
```

#### 4.5.2 获取IMU数据

**接口调用**: `Query(QueryRequest)`
**功能标识**: `input.action = SENSOR_GET_IMU_DATA`

**输出结果 (QueryResponse.output)**:

```yaml
imu_data:
  header:
    timestamp: 1672531200000
    frame_id: "imu_link"
  orientation: {x: 0.0, y: 0.0, z: 0.0, w: 1.0}
  angular_velocity: {x: 0.01, y: 0.02, z: 0.0}
  linear_acceleration: {x: 0.0, y: 0.0, z: 9.81}
  orientation_covariance: [0.01, 0.0, 0.0, 0.0, 0.01, 0.0, 0.0, 0.0, 0.01]
imu_id: "body_imu"
is_calibrated: true
```

#### 4.5.3 获取TF变换

**接口调用**: `Query(QueryRequest)`
**功能标识**: `input.action = SENSOR_GET_TF_TRANSFORMS`

**输出结果 (QueryResponse.output)**:

```yaml
transforms:
  - header:
      timestamp: 1672531200000
      frame_id: "map"
    child_frame_id: "base_link"
    transform:
      translation: {x: 1.0, y: 2.0, z: 0.0}
      rotation: {x: 0.0, y: 0.0, z: 0.0, w: 1.0}
base_frame: "map"
timestamp: 1672531200000
```

## 5. 订阅机制

### 5.1 持久订阅接口

```protobuf
message SubscribeRequest {
  string objectId = 1;                   // 订阅对象ID
  string clientEndpoint = 2;             // 客户端回调地址  
  bool persistentSubscription = 3;       // 是否持久订阅
  repeated string eventTypes = 4;        // 事件类型过滤
  int32 heartbeatInterval = 5;           // 心跳间隔(秒)
  base_types.Dictionary params = 6;      // 其他参数
}
```

### 5.2 支持的订阅类型

通过 `eventTypes` 字段指定：

- **"sensor.camera"**: 相机数据流
- **"sensor.imu"**: IMU数据流  
- **"sensor.lidar"**: 激光雷达数据流
- **"navigation.pose"**: 机器人位姿更新
- **"navigation.status"**: 导航状态变化
- **"perception.detection"**: 检测结果更新
- **"task.status"**: 任务状态变化
- **"error.critical"**: 严重错误通知

### 5.3 订阅消息格式

订阅推送的消息使用相同的 `SubscribeResponse` 格式：

```yaml
output:
  event_type: "sensor.camera"  # 事件类型
  object_id: "front_camera"    # 对象ID
  data:                        # 具体数据内容
    # ... 相机数据或其他传感器数据
  timestamp: 1672531200000
ret: 0                         # 0=正常推送
```

## 6. 性能要求

### 6.1 延迟要求

- **实时传感器数据**: < 50ms
- **导航指令响应**: < 100ms
- **感知结果返回**: < 200ms
- **控制指令执行**: < 10ms

### 6.2 吞吐量要求

- **图像数据传输**: 支持30fps的高清图像流
- **并发连接数**: 支持最少50个并发客户端
- **订阅管理**: 支持1000个持久订阅

### 6.3 可靠性要求

- **系统可用性**: 99.9%
- **数据完整性**: 100%
- **故障恢复时间**: < 5秒

## 7. 部署要求

### 7.1 硬件要求

- **CPU**: 至少4核，推荐8核以上
- **内存**: 至少8GB，推荐16GB以上
- **网络**: 千兆以太网，延迟<1ms
- **存储**: 至少100GB可用空间

### 7.2 软件环境

- **操作系统**: Ubuntu 20.04 LTS 或更高版本
- **gRPC版本**: 1.71.0 或更高版本
- **Protobuf版本**: 5.29.3 或更高版本
- **编译器**: GCC 9.0 或更高版本，支持C++17

### 7.3 配置管理

- 支持YAML配置文件
- 支持环境变量配置
- 支持运行时配置更新
- 支持配置版本管理

## 8. 安全要求

### 8.1 通信安全

- 支持TLS加密通信
- 支持客户端证书认证
- 支持API密钥认证

### 8.2 访问控制

- 基于角色的访问控制(RBAC)
- API调用频率限制
- IP白名单机制

## 9. 监控与日志

### 9.1 监控指标

- 接口调用次数和延迟
- 系统资源使用情况
- 错误率和成功率统计
- 活跃订阅数量

### 9.2 日志管理

- 结构化日志输出
- 日志级别控制
- 日志轮转和归档
- 分布式日志聚合

## 10. 测试要求

### 10.1 单元测试

- 代码覆盖率 > 80%
- 所有公共接口都有对应测试
- 模拟依赖项进行隔离测试

### 10.2 集成测试

- 端到端功能测试
- 性能压力测试
- 故障注入测试
- 多客户端并发测试

### 10.3 验收测试

- 用户场景测试
- 兼容性测试
- 安全性测试
- 可用性测试

## 11. 维护与支持

### 11.1 版本管理

- 语义化版本号
- 向后兼容性保证
- 变更日志维护
- 迁移指南提供

### 11.2 技术支持

- API文档维护
- 示例代码提供
- 问题跟踪系统
- 社区支持渠道

---

**文档版本**: v1.0  
**创建日期**: 2025年8月4日  
**最后更新**: 2025年8月4日  
**维护人员**: 人形机器人项目组
```
