#ifndef MODBUSREQUESTQUEUE_H
#define MODBUSREQUESTQUEUE_H

#include <QModbusDataUnit>
#include <QQueue>
#include <QString>
#include <QVector>

enum class ModbusRequestType
{
    Read,
    Write
};

struct ModbusRequest
{
    ModbusRequestType type = ModbusRequestType::Read;
    int slaveId = 1;
    QModbusDataUnit::RegisterType registerType = QModbusDataUnit::HoldingRegisters;
    int startAddress = 0;
    quint16 quantity = 0;//数量
    QVector<quint16> values;
    quint64 requestId = 0;//请求ID
    int retryCount = 0;//重试次数

    /** 生成用于读请求去重和写请求替换的唯一键。 */
    QString key() const;
};

class ModbusRequestQueue
{
public:
    enum class ResultType
    {
        Added,//添加
        Duplicate,//重复
        Replaced//替换
    };

    struct Result
    {
        /** 记录入队操作对当前逻辑请求的处理结果。 */
        Result(ResultType resultType = ResultType::Added,
               quint64 effectiveId = 0,
               quint64 replacedId = 0)
            : type(resultType)
            , effectiveRequestId(effectiveId)
            , replacedRequestId(replacedId)
        {
        }

        ResultType type = ResultType::Added;
        quint64 effectiveRequestId = 0;
        quint64 replacedRequestId = 0;
    };

    /**
     * 读请求按 FIFO 顺序入队，写请求插入队首。
     * 入队前执行请求去重，并用最新写入值替换尚未发送的旧值。
     */
    Result enqueue(const ModbusRequest &request);

    /** 将同一逻辑请求插入队首，供管理器执行下一次重试。 */
    void prependRetry(const ModbusRequest &request);

    /** 队列中不存在未发送请求时返回 true。 */
    bool isEmpty() const;

    /** 返回当前未发送请求数量。 */
    int size() const;

    /** 按当前优先级移除并返回下一个待发送请求。 */
    ModbusRequest dequeue();

    /** 按执行顺序取出全部请求，主要用于断线清理。 */
    QList<ModbusRequest> takeAll();

    /** 直接清空全部未发送请求，不触发完成通知。 */
    void clear();

private:
    QQueue<ModbusRequest> m_queue;
};

#endif // MODBUSREQUESTQUEUE_H
