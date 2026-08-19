# Sony Aquaculture

基于 Qt Widgets 开发的水产养殖监测与控制上位机，使用 Modbus RTU 采集传感器
数据，提供实时趋势、设备状态、远程控制、故障检测和日志查询等功能。

## 技术栈

- C++11
- Qt 5.15 Widgets
- Qt SerialBus、Qt SerialPort
- Qt Charts
- QMYSQL
- qmake、MinGW

## 主要功能

- 根据 Modbus 配置一次轮询 13 项保持寄存器数据
- 基于请求队列实现读请求 FIFO、控制写请求优先、重复请求去重和待发送写请求的最新值替换
- 将 Modbus 通信和响应解析放在工作线程，避免串口 I/O 阻塞 UI 线程
- 实现请求级超时重试与串口断线后的阶梯退避重连
- 使用 Online、Unstable、Error、Offline 四态描述从站健康状态
- 使用 Good、Stale、Bad 表示采集数据质量并联动界面显示
- 实时曲线采用固定长度滑动窗口，避免数据点持续增长
- 支持投喂、温控、增氧、水位等设备的远程控制
- 提供故障检测、告警提示和异常日志查询页面

## 构建方式

使用 Qt 5.15 MinGW Kit 打开 `Sony-Aquaculture.pro`，通过 qmake 构建。
项目依赖 `lib/` 下的滑动按钮库和 `ptr/` 下的运行时图片资源。

公开配置中的默认串口为 `COM1`。请根据实际串口和从站配置修改
`modbus.ini`；其中 Modbus 地址使用从 0 开始的 PDU 地址。

## 数据库配置

数据库仅用于数据存储，采集链路不依赖其持续可用。请通过环境变量配置
数据库连接，避免把凭据写入源码：

```text
DY_DB_HOST       （默认：127.0.0.1）
DY_DB_USER       （默认：root）
DY_DB_PASSWORD   （MySQL 访问所需）
```

默认数据库名为 `test`。

## 测试

Modbus 单元测试位于 `tests/`，建议使用 Qt 5.15 Kit 在独立构建目录中执行：

```sh
qmake ../SonyAquaculture-main/tests/modbus_tests.pro
mingw32-make
```

本仓库为脱敏后的公开副本，未包含凭据和本机生成文件。
