#ifndef MONITOR_TYPES_H
#define MONITOR_TYPES_H

#include <QDateTime>
#include <QList>
#include <QMetaType>
#include <QString>
#include <QVector>

//监控点
enum class MonitorPoint
{
    Temp = 0,
    Humidity,
    Pressure,
    Voc,
    Light,
    Ph,
    Tds,
    Ec,
    Turbidity,
    Temp1,
    Temp2,
    WaterLevel1,
    WaterLevel2,
    Count
};

//数据质量
enum class DataQuality
{
    Good,
    Stale,//旧的
    Bad
};

enum class DeviceState
{
    Online,//在线
    Unstable,//不稳定
    Error,  //错误
    Offline//离线
};

enum class ConnectionState
{
    Disconnected,
    Connecting,
    Connected
};

//设备统计
struct DeviceStatistics
{
    int slaveId = 0;
    DeviceState state = DeviceState::Offline;
    quint64 successCount = 0; //成功计数
    quint64 errorCount = 0;
    int consecutiveTimeouts = 0; //连续超时
    QDateTime lastSuccessTime; //上次成功时间
};

//监控快照
struct MonitorSnapshot
{
    /** 创建包含 13 项监测点且尚无有效采样值的快照。 */
    MonitorSnapshot()
        : values(static_cast<int>(MonitorPoint::Count), 0.0)
        , qualities(static_cast<int>(MonitorPoint::Count), DataQuality::Bad)
        , valueTimes(static_cast<int>(MonitorPoint::Count))
    {
    }

    /** 返回指定监测点保存的工程量数值。 */
    double value(MonitorPoint point) const
    {
        return values.value(static_cast<int>(point));
    }

    /** 返回指定监测点当前的 Good、Stale 或 Bad 数据质量。 */
    DataQuality quality(MonitorPoint point) const
    {
        return qualities.value(static_cast<int>(point), DataQuality::Bad);
    }

    /** 保存成功解析的数值，并将数据质量标记为 Good。 */
    void setGood(MonitorPoint point, double newValue, const QDateTime &time)
    {
        const int index = static_cast<int>(point);
        values[index] = newValue;
        qualities[index] = DataQuality::Good;
        valueTimes[index] = time;
    }

    /** 保留最后有效值，仅将已有 Good 采样标记为 Stale。 */
    void markStale(MonitorPoint point)
    {
        const int index = static_cast<int>(point);
        if (qualities[index] == DataQuality::Good)
            qualities[index] = DataQuality::Stale;
    }

    QVector<double> values;
    QVector<DataQuality> qualities;
    QVector<QDateTime> valueTimes;
    QDateTime timestamp;
    qint64 responseTimestampMs = 0;
    QList<DeviceStatistics> devices;
};

Q_DECLARE_METATYPE(MonitorPoint)
Q_DECLARE_METATYPE(DataQuality)
Q_DECLARE_METATYPE(DeviceState)
Q_DECLARE_METATYPE(ConnectionState)
Q_DECLARE_METATYPE(DeviceStatistics)
Q_DECLARE_METATYPE(MonitorSnapshot)


/*
Q_DECLARE_METATYPE(Type) 是 Qt 元对象系统 中的一个核心宏，
它的作用是将自定义类型（C++ 类、结构体、枚举等）注册到 Qt 的元类型系统（Meta-Type System）中，使得该类型能够被 Qt 的某些高级特性识别和使用。

1. 为什么需要它？（核心使用场景）
没有注册的类型，在以下两个关键场景中无法正常工作：

存储在 QVariant 中：QVariant 是 Qt 的万能数据类型容器。只有注册过的类型，才能使用 QVariant::fromValue(myType) 存入，以及用 qvariant.value<MyType>() 取出。

在信号槽（Signal & Slot）中传递：特别是跨线程的队列连接（Queued Connection）。
当信号和槽在不同线程时，参数需要被序列化（拷贝）到事件队列中，Qt 必须知道该类型的内存大小和构造方式，这依赖于元类型系统。
*/
#endif // MONITOR_TYPES_H
