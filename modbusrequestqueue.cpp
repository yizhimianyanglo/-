#include "modbusrequestqueue.h"

QString ModbusRequest::key() const
{
    return QStringLiteral("%1:%2:%3:%4:%5")
        .arg(static_cast<int>(type))
        .arg(slaveId)
        .arg(static_cast<int>(registerType))
        .arg(startAddress)
        .arg(quantity);
}

ModbusRequestQueue::Result ModbusRequestQueue::enqueue(const ModbusRequest &request)
{
    // 去重后队列长度较短，直接扫描可保持实现简单；替换操作也能在通信线程内原子完成。
    for (int index = 0; index < m_queue.size(); ++index)
    {
        const ModbusRequest &queued = m_queue.at(index);
        if (queued.key() != request.key())
            continue;

        if (request.type == ModbusRequestType::Read || queued.values == request.values) {
            return {ResultType::Duplicate, queued.requestId, 0};
        }

        // 尚未发送的旧控制值没有到达设备，只保留最新值并插入队首；在途请求不受影响。
        const quint64 replacedId = queued.requestId;
        m_queue.removeAt(index);
        m_queue.prepend(request);
        return {ResultType::Replaced, request.requestId, replacedId};
    }

    // 读请求保持 FIFO 顺序，控制写请求优先于所有尚未发送的请求。
    if (request.type == ModbusRequestType::Write)
        m_queue.prepend(request);
    else
        m_queue.enqueue(request);

    return {ResultType::Added, request.requestId, 0};
}

void ModbusRequestQueue::prependRetry(const ModbusRequest &request)
{
    m_queue.prepend(request);
}

bool ModbusRequestQueue::isEmpty() const
{
    return m_queue.isEmpty();
}

int ModbusRequestQueue::size() const
{
    return m_queue.size();
}

ModbusRequest ModbusRequestQueue::dequeue()
{
    return m_queue.dequeue();
}

QList<ModbusRequest> ModbusRequestQueue::takeAll()
{
    QList<ModbusRequest> requests;
    while (!m_queue.isEmpty())
        requests.append(m_queue.dequeue());
    return requests;
}

void ModbusRequestQueue::clear()
{
    m_queue.clear();
}
