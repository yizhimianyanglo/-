# 水产养殖自动监测与控制上位机

这是项目的公开发布仓库。完整 Qt 源码位于
[`SonyAquaculture-main/`](SonyAquaculture-main/) 子目录，仓库根目录同时保留
Modbus Slave 模拟器文件、通信设计说明和项目文档。

## 项目简介

基于 C++ 和 Qt 5.15 Widgets 开发水产养殖上位机，实现 Modbus RTU 串口采集、
实时曲线展示、设备状态管理、控制指令下发、故障检测、告警提示和日志查询。

## 目录结构

```text
SonyAquaculture-main/
├── Sony-Aquaculture.pro     Qt 工程文件
├── modbus.ini               运行时串口、点表和轮询配置
├── modbusmanger.*            Modbus 通信线程与请求调度
├── modbusrequestqueue.*      请求队列
├── monitor_types.h           监测数据和状态类型
├── tests/                    Modbus 单元测试
├── ptr/                      界面图片资源
└── include/、lib/            滑动按钮库
```

## 主要功能

- 一次轮询采集 13 项传感器保持寄存器数据
- 读请求 FIFO、控制写请求队首插队、重复请求去重和最新值替换
- 工作线程执行串口 I/O 和响应解析，主线程负责 UI 更新
- 请求级超时重试、1s/2s/4s/30s 串口断线重连
- Online、Unstable、Error、Offline 四态从站健康模型
- Good、Stale、Bad 数据质量状态与曲线快照保留
- 实时曲线、故障检测、告警、远程控制和日志查询

## 构建与配置

使用 Qt 5.15 MinGW Kit 打开 `SonyAquaculture-main/Sony-Aquaculture.pro`，
通过 qmake 构建。公开配置默认使用 `COM1`，实际使用时请修改
`SonyAquaculture-main/modbus.ini`。

数据库连接通过环境变量配置，避免将凭据写入源码：

```text
DY_DB_HOST       （默认：127.0.0.1）
DY_DB_USER       （默认：root）
DY_DB_PASSWORD   （MySQL 访问所需）
```

## 项目文档

- [Modbus RTU 实现方案](Modbus_RTU_Implementation_Plan.md)
- [项目通信说明](SonyProjectModbus.md)
- [数据库初始化脚本](SonyAquaculture-main/Aquaculture.sql)

本仓库为脱敏后的公开副本，不包含数据库密码、本机绝对路径和生成的可执行文件。
