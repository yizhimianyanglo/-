#ifndef MODBUSMANGER_H
#define MODBUSMANGER_H

#include "modbusrequestqueue.h"
#include "monitor_types.h"

#include <QHash>
#include <QMap>
#include <QModbusDevice>
#include <QObject>
#include <QString>

class QModbusReply;
class QModbusRtuSerialMaster;
class QTimer;

class ModbusManager : public QObject
{
    Q_OBJECT

public:
    /** 创建通信线程内使用的管理器，并记录可选的 INI 配置文件路径。 */
    explicit ModbusManager(const QString &configFilePath, QObject *parent = nullptr);

    /** 释放由管理器通信线程持有的对象。 */
    ~ModbusManager() override;

    /** 根据连续超时次数计算设备的四级运行状态。 */
    static DeviceState stateForConsecutiveTimeouts(int timeoutCount);

    /** 按 1/2/4/30 秒阶梯返回指定重连次数对应的等待时间。 */
    static int reconnectDelayForAttempt(int attempt);

    /** 判断当前已完成的重试次数是否仍允许再次发送。 */
    static bool shouldRetry(int retryCount, int maximumRetries);

    /**
     * 按配置的数据类型和字节序解析一个 Modbus 数值，并应用工程量除数。
     * 数据类型、字节序或寄存器数量不合法时，通过 ok 返回 false。
     */
    static double decodeValue(const QVector<quint16> &registers,
                              const QString &dataType,
                              double divisor,
                              const QString &byteOrder,
                              bool *ok);

signals:
    /** 数据值或质量变化后，立即按值传递最新监测快照。 */
    void snapshotReady(const MonitorSnapshot &snapshot);

    /** 通知串口连接状态变化，并携带诊断信息。 */
    void connectionStateChanged(ConnectionState state, const QString &detail);

    /** 通知从站在 Online、Unstable、Error、Offline 状态之间切换。 */
    void slaveStateChanged(int slaveId, DeviceState state, const QString &detail);

    /** 返回控制写入结果，同时通知因写请求去重而合并的调用方。 */
    void writeRequestFinished(quint64 requestId, bool success, const QString &detail);

    /** 报告最终失败的读写请求，供日志记录和操作员反馈使用。 */
    void requestFailed(quint64 requestId, const QString &detail);

    /** 报告 modbus.ini 缺失或配置项不合法。 */
    void configurationError(const QString &detail);

public slots:
    /** 加载配置，初始化串口客户端和定时器，并发起首次连接。 */
    void start();

    /** 停止轮询和重连，并将无法安全完成的请求置为失败。 */
    void stop();

    /** 将指定控制项写请求高优先级入队，实际发送始终在通信线程执行。 */
    void enqueueControlWrite(const QString &controlName, quint16 value, quint64 requestId);

private slots:
    /** 根据各从站当前轮询频率，将到期的读块加入请求队列。 */
    void handlePolling();

    /** 客户端处于断开状态时，发起一次异步串口连接。 */
    void attemptConnect();

    /** 仅处理一次当前 QModbusReply，并在完成后推进请求队列。 */
    void handleReplyFinished();

private:
    //点定义
    struct PointDefinition
    {
        MonitorPoint point = MonitorPoint::Temp;
        int slaveId = 1;
        QModbusDataUnit::RegisterType registerType = QModbusDataUnit::HoldingRegisters;
        int address = 0;
        int registerCount = 1;
        QString dataType = QStringLiteral("UInt16");
        QString byteOrder = QStringLiteral("AB");//字节顺序
        double divisor = 1.0;
    };

    //控制定义
    struct ControlDefinition
    {
        int slaveId = 1;
        QModbusDataUnit::RegisterType registerType = QModbusDataUnit::HoldingRegisters;
        int address = 0;
    };

    struct ReadBlock
    {
        int slaveId = 1;
        QModbusDataUnit::RegisterType registerType = QModbusDataUnit::HoldingRegisters;
        int startAddress = 0;
        int quantity = 0;
    };

    //运行设备
    struct RuntimeDevice
    {
        DeviceStatistics statistics; //设备统计
        qint64 nextPollAtMs = 0;
    };

    /** 从 INI 加载并校验串口、轮询、监测点和控制点配置。 */
    bool loadConfiguration();

    /** 合并相邻监测点，生成单次最多 125 个寄存器的合法 Modbus 读块。 */
    void buildReadBlocks();

    /** 应用串口参数并关闭 Qt 内置重试，确保请求重试由本模块统一管理。 */
    void configureClient();

    /** 按当前退避级别启动单次重连定时器。 */
    void scheduleReconnect();

    /** 连接成功后重置退避次数，并使所有已配置从站可立即轮询。 */
    void handleConnected();

    /** 断线时终止未完成写请求、清理读请求、标记陈旧值并安排重连。 */
    void handleTransportDisconnected(const QString &detail);

    /** 将一个配置读块转换为支持去重的 FIFO 读请求。 */
    void enqueueReadBlock(const ReadBlock &block);

    /** 串口已连接且当前无在途请求时，发送队首请求。 */
    void sendNextRequest();

    /** 对当前请求执行重试或最终失败处理。 */
    void finishCurrentRequest(QModbusDevice::Error error, const QString &detail);

    /** 断开当前 reply 的信号连接，延迟释放对象并清空指针。 */
    void clearCurrentReply();

    /** 解析成功读块覆盖的全部监测点，并发布最新快照。 */
    void handleReadSuccess(const ModbusRequest &request, const QModbusDataUnit &result);

    /** 读请求失败时保留最后有效值，并将对应监测点标记为陈旧。 */
    void handleReadFailure(const ModbusRequest &request);

    /** 向实际写请求及所有去重别名发送同一写入结果。 */
    void completeWriteRequest(quint64 requestId, bool success, const QString &detail);

    /** 记录成功响应，并立即将从站恢复为 Online 状态。 */
    void updateDeviceSuccess(int slaveId);

    /** 记录请求失败，并按连续超时更新设备状态和轮询频率。 */
    void updateDeviceFailure(int slaveId, bool timeout, const QString &detail);

    /** 将当前设备统计信息写入快照并发布。 */
    void emitSnapshot();

    /** 返回设备当前状态对应的探测轮询间隔。 */
    int pollIntervalForState(DeviceState state) const;

    /** 查找显式指定路径或程序约定位置中的 modbus.ini。 */
    QString resolveConfigPath() const;

    /** 将 INI 监测点分组名称转换为固定的 13 项枚举。 */
    static bool pointFromName(const QString &name, MonitorPoint *point);

    /** 将 INI 寄存器区名称转换为对应的 Qt 寄存器类型。 */
    static QModbusDataUnit::RegisterType registerTypeFromName(const QString &name, bool *ok);

private:

    QString m_configFilePath;
    QModbusRtuSerialMaster *m_client = nullptr;
    QModbusReply *m_currentReply = nullptr;
    QTimer *m_pollTimer = nullptr;
    QTimer *m_reconnectTimer = nullptr;
    ModbusRequestQueue m_requestQueue;
    ModbusRequest m_currentRequest;
    bool m_hasCurrentRequest = false;
    bool m_stopping = false;
    bool m_disconnectHandled = true;
    quint64 m_nextRequestId = 1;
    int m_reconnectAttempt = 0;

    QString m_portName = QStringLiteral("COM1");
    int m_baudRate = 9600;
    int m_dataBits = 8;
    int m_stopBits = 1;
    QString m_parity = QStringLiteral("Even");
    int m_pollIntervalMs = 1000;
    int m_requestTimeoutMs = 3000;
    int m_maxRetries = 3;

    QVector<PointDefinition> m_points;
    QList<ReadBlock> m_readBlocks;
    QMap<QString, ControlDefinition> m_controls;
    QMap<int, RuntimeDevice> m_devices;// key:slaveId   value:RuntimeDevice
    QHash<quint64, QList<quint64>> m_requestAliases;//请求别名
    MonitorSnapshot m_snapshot;
};

#endif // MODBUSMANGER_H
