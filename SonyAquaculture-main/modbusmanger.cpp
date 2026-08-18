#include "modbusmanger.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QModbusDevice>
#include <QModbusReply>
#include <QModbusRtuSerialMaster>
#include <QSerialPort>
#include <QSet>
#include <QSettings>
#include <QThread>
#include <QTimer>

#include <algorithm>
#include <cstring>

namespace {

// 返回配置标量类型占用的 Modbus 寄存器数量。
int registerCountForType(const QString &dataType)
{
    const QString normalized = dataType.trimmed().toLower();
    if (normalized == QStringLiteral("uint16") || normalized == QStringLiteral("int16"))
        return 1;
    if (normalized == QStringLiteral("uint32") || normalized == QStringLiteral("int32")
        || normalized == QStringLiteral("float32")) {
        return 2;
    }
    return 0;
}

// 将设备状态枚举转换为诊断信号使用的文本。
QString deviceStateText(DeviceState state)
{
    switch (state) {
    case DeviceState::Online:
        return QStringLiteral("Online");
    case DeviceState::Unstable:
        return QStringLiteral("Unstable");
    case DeviceState::Error:
        return QStringLiteral("Error");
    case DeviceState::Offline:
        return QStringLiteral("Offline");
    }
    return QStringLiteral("Unknown");
}

} // namespace

ModbusManager::ModbusManager(const QString &configFilePath, QObject *parent)
    : QObject(parent)
    , m_configFilePath(configFilePath)
{
}

ModbusManager::~ModbusManager() = default;

DeviceState ModbusManager::stateForConsecutiveTimeouts(int timeoutCount)
{
    if (timeoutCount >= 10)
        return DeviceState::Offline;
    if (timeoutCount >= 5)
        return DeviceState::Error;
    if (timeoutCount >= 3)
        return DeviceState::Unstable;
    return DeviceState::Online;
}

int ModbusManager::reconnectDelayForAttempt(int attempt)
{
    static const int delays[] = {1000, 2000, 4000, 30000};
    return delays[qBound(0, attempt, 3)];
}

bool ModbusManager::shouldRetry(int retryCount, int maximumRetries)
{
    // retryCount 不包含首次发送，因此 maxRetries=3 表示总共最多发送 4 次。
    return retryCount < maximumRetries;
}

double ModbusManager::decodeValue(const QVector<quint16> &registers,
                                  const QString &dataType,
                                  double divisor,
                                  const QString &byteOrder,
                                  bool *ok)
{
    if (ok)
        *ok = false;
    if (qFuzzyIsNull(divisor))
        return 0.0;

    const QString type = dataType.trimmed().toLower();
    const int registerCount = registerCountForType(type);
    if (registerCount == 0 || registers.size() < registerCount)
        return 0.0;

    QByteArray raw;
    raw.reserve(registerCount * 2);
    for (int index = 0; index < registerCount; ++index) {
        raw.append(static_cast<char>((registers.at(index) >> 8) & 0xff));
        raw.append(static_cast<char>(registers.at(index) & 0xff));
    }

    const QString expectedOrder = registerCount == 1 ? QStringLiteral("AB")
                                                       : QStringLiteral("ABCD");
    QString order = byteOrder.trimmed().toUpper();
    if (order.isEmpty())
        order = expectedOrder;
    if (order.size() != expectedOrder.size())
        return 0.0;

    // 将设备配置的字节序转换为统一的大端 ABCD 顺序。
    QByteArray canonical(raw.size(), '\0');
    for (int target = 0; target < expectedOrder.size(); ++target) {
        const int source = order.indexOf(expectedOrder.at(target));
        if (source < 0 || source >= raw.size())
            return 0.0;
        canonical[target] = raw.at(source);
    }

    double decoded = 0.0;
    if (registerCount == 1) {
        const quint16 unsignedValue = (static_cast<quint8>(canonical.at(0)) << 8)
                                      | static_cast<quint8>(canonical.at(1));
        decoded = type == QStringLiteral("int16") ? static_cast<qint16>(unsignedValue)
                                                   : unsignedValue;
    } else {
        quint32 unsignedValue = 0;
        for (char byte : canonical)
            unsignedValue = (unsignedValue << 8) | static_cast<quint8>(byte);

        if (type == QStringLiteral("int32")) {
            decoded = static_cast<qint32>(unsignedValue);
        } else if (type == QStringLiteral("float32")) {
            float floatValue = 0.0f;
            std::memcpy(&floatValue, &unsignedValue, sizeof(floatValue));
            decoded = floatValue;
        } else {
            decoded = unsignedValue;
        }
    }

    if (ok)
        *ok = true;
    return decoded / divisor;
}

void ModbusManager::start()
{
    if (m_client || m_stopping)
        return;

    if (!loadConfiguration())
        return;

    m_client = new QModbusRtuSerialMaster(this);
    configureClient();

    connect(m_client, &QModbusDevice::stateChanged, this,
            [this](QModbusDevice::State state) {
                if (state == QModbusDevice::ConnectedState)
                    handleConnected();
                else if (state == QModbusDevice::UnconnectedState) {
                    emit connectionStateChanged(ConnectionState::Disconnected,
                                                m_client ? m_client->errorString() : QString());
                    handleTransportDisconnected(m_client ? m_client->errorString()
                                                         : QStringLiteral("Serial port disconnected"));
                }
            });

    m_pollTimer = new QTimer(this);
    m_pollTimer->setInterval(100);
    connect(m_pollTimer, &QTimer::timeout, this, &ModbusManager::handlePolling);
    m_pollTimer->start();

    m_reconnectTimer = new QTimer(this);//重新连接定时器
    m_reconnectTimer->setSingleShot(true);
    connect(m_reconnectTimer, &QTimer::timeout, this, &ModbusManager::attemptConnect);//尝试连接

    attemptConnect();
}

void ModbusManager::stop()
{
    if (m_stopping)
        return;

    m_stopping = true;
    if (m_pollTimer)
        m_pollTimer->stop();
    if (m_reconnectTimer)
        m_reconnectTimer->stop();

    handleTransportDisconnected(QStringLiteral("Modbus worker stopped"));
    if (m_client && m_client->state() != QModbusDevice::UnconnectedState)
        m_client->disconnectDevice();
}

QString ModbusManager::resolveConfigPath() const
{
    if (!m_configFilePath.isEmpty() && QFileInfo::exists(m_configFilePath))
        return m_configFilePath;

    const QStringList candidates = {
        QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("modbus.ini")),
        QDir::current().filePath(QStringLiteral("modbus.ini")),
        QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("../modbus.ini"))
    };
    for (const QString &candidate : candidates) {
        if (QFileInfo::exists(candidate))
            return QFileInfo(candidate).absoluteFilePath();
    }
    return QString();
}

bool ModbusManager::loadConfiguration()
{
    const QString configPath = resolveConfigPath();
    if (configPath.isEmpty()) {
        emit configurationError(QStringLiteral("modbus.ini was not found"));
        return false;
    }

    QSettings settings(configPath, QSettings::IniFormat);
    settings.setIniCodec("UTF-8");

    settings.beginGroup(QStringLiteral("Serial"));
    m_portName = settings.value(QStringLiteral("port"), QStringLiteral("COM1")).toString();
    m_baudRate = settings.value(QStringLiteral("baudRate"), 9600).toInt();
    m_parity = settings.value(QStringLiteral("parity"), QStringLiteral("Even")).toString();
    m_dataBits = settings.value(QStringLiteral("dataBits"), 8).toInt();
    m_stopBits = settings.value(QStringLiteral("stopBits"), 1).toInt();
    settings.endGroup();

    settings.beginGroup(QStringLiteral("Polling"));
    m_pollIntervalMs = settings.value(QStringLiteral("pollIntervalMs"), 1000).toInt();
    m_requestTimeoutMs = settings.value(QStringLiteral("requestTimeoutMs"), 3000).toInt();
    m_maxRetries = settings.value(QStringLiteral("maxRetries"), 3).toInt();
    settings.endGroup();

    if (m_portName.isEmpty() || m_pollIntervalMs <= 0 || m_requestTimeoutMs <= 0
        || m_maxRetries < 0) {
        emit configurationError(QStringLiteral("Invalid serial or polling configuration"));
        return false;
    }

    m_points.clear();
    QSet<int> configuredPoints;
    // 使用完整键名遍历 QSettings，稳定识别 INI 中的嵌套分组。
    const auto subgroupNames = [&settings](const QString &prefix) {
        QSet<QString> groups;
        const QString keyPrefix = prefix + QLatin1Char('/');
        for (const QString &key : settings.allKeys()) {
            if (key.startsWith(keyPrefix))
                groups.insert(key.mid(keyPrefix.size()).section(QLatin1Char('/'), 0, 0));
        }
        return groups.values();
    };
    const QStringList pointGroups = subgroupNames(QStringLiteral("Points"));
    for (const QString &group : pointGroups) {
        MonitorPoint point;
        if (!pointFromName(group, &point))
            continue;

        settings.beginGroup(QStringLiteral("Points/%1").arg(group));
        PointDefinition definition;
        definition.point = point;
        definition.slaveId = settings.value(QStringLiteral("slave"), 1).toInt();
        bool registerTypeOk = false;
        definition.registerType = registerTypeFromName(
            settings.value(QStringLiteral("table"), QStringLiteral("HoldingRegisters")).toString(),
            &registerTypeOk);
        definition.address = settings.value(QStringLiteral("address"), -1).toInt();
        definition.dataType = settings.value(QStringLiteral("dataType"), QStringLiteral("UInt16")).toString();
        definition.registerCount = registerCountForType(definition.dataType);
        definition.byteOrder = settings.value(QStringLiteral("byteOrder"),
                                              definition.registerCount == 1 ? QStringLiteral("AB")
                                                                            : QStringLiteral("ABCD")).toString();
        definition.divisor = settings.value(QStringLiteral("divisor"), 1.0).toDouble();
        settings.endGroup();

        if (!registerTypeOk
            || (definition.registerType != QModbusDataUnit::HoldingRegisters
                && definition.registerType != QModbusDataUnit::InputRegisters)
            || definition.slaveId < 1 || definition.slaveId > 247
            || definition.address < 0 || definition.address + definition.registerCount > 65536
            || definition.registerCount == 0 || qFuzzyIsNull(definition.divisor)
            || configuredPoints.contains(static_cast<int>(point))) {
            emit configurationError(QStringLiteral("Invalid point definition: %1").arg(group));
            return false;
        }

        configuredPoints.insert(static_cast<int>(point));
        m_points.append(definition);
    }
    if (configuredPoints.size() != static_cast<int>(MonitorPoint::Count)) {
        emit configurationError(QStringLiteral("All 13 monitor points must be configured"));
        return false;
    }

    m_controls.clear();
    const QStringList controlGroups = subgroupNames(QStringLiteral("Controls"));
    for (const QString &group : controlGroups) {
        settings.beginGroup(QStringLiteral("Controls/%1").arg(group));
        ControlDefinition definition;
        definition.slaveId = settings.value(QStringLiteral("slave"), 1).toInt();
        bool registerTypeOk = false;
        definition.registerType = registerTypeFromName(
            settings.value(QStringLiteral("table"), QStringLiteral("HoldingRegisters")).toString(),
            &registerTypeOk);
        definition.address = settings.value(QStringLiteral("address"), -1).toInt();
        settings.endGroup();

        if (!registerTypeOk
            || (definition.registerType != QModbusDataUnit::Coils
                && definition.registerType != QModbusDataUnit::HoldingRegisters)
            || definition.slaveId < 1 || definition.slaveId > 247
            || definition.address < 0 || definition.address > 65535) {
            emit configurationError(QStringLiteral("Invalid control definition: %1").arg(group));
            return false;
        }
        m_controls.insert(group, definition);
    }
    const QStringList requiredControls = {
        QStringLiteral("FeedingAuto"), QStringLiteral("TempAuto"),
        QStringLiteral("WaterLevelAuto"), QStringLiteral("EnvironmentAuto"),
        QStringLiteral("OxygenAuto"), QStringLiteral("SolarAuto"),
        QStringLiteral("FeedingFrequency"), QStringLiteral("FeedingAmount"),
        QStringLiteral("WaterFlow"), QStringLiteral("PowerMode"),
        QStringLiteral("CommunicationMode"), QStringLiteral("OxygenDuration"),
        QStringLiteral("OxygenFrequency")
    };
    for (const QString &control : requiredControls) {
        if (!m_controls.contains(control)) {
            emit configurationError(QStringLiteral("Missing control definition: %1").arg(control));
            return false;
        }
    }

    buildReadBlocks();
    m_devices.clear();
    for (const PointDefinition &point : m_points) {
        RuntimeDevice &device = m_devices[point.slaveId];
        device.statistics.slaveId = point.slaveId;
        device.statistics.state = DeviceState::Offline;
    }
    return !m_readBlocks.isEmpty();
}

void ModbusManager::buildReadBlocks()
{
    QVector<PointDefinition> sortedPoints = m_points;
    std::sort(sortedPoints.begin(), sortedPoints.end(),
              [](const PointDefinition &left, const PointDefinition &right) {
                  if (left.slaveId != right.slaveId)
                      return left.slaveId < right.slaveId;
                  if (left.registerType != right.registerType)
                      return left.registerType < right.registerType;
                  return left.address < right.address;
              });

    m_readBlocks.clear();
    for (const PointDefinition &point : sortedPoints)
    {
        if (!m_readBlocks.isEmpty())
        {
            ReadBlock &last = m_readBlocks.last();
            const int lastEnd = last.startAddress + last.quantity;
            const int pointEnd = point.address + point.registerCount;
            if (last.slaveId == point.slaveId && last.registerType == point.registerType
                && point.address <= lastEnd && pointEnd - last.startAddress <= 125)
            {
                last.quantity = qMax(lastEnd, pointEnd) - last.startAddress;
                continue;
            }
        }

        ReadBlock block;
        block.slaveId = point.slaveId;
        block.registerType = point.registerType;
        block.startAddress = point.address;
        block.quantity = point.registerCount;
        m_readBlocks.append(block);
    }
}

void ModbusManager::configureClient()
{
    QSerialPort::Parity parity = QSerialPort::EvenParity;
    if (m_parity.compare(QStringLiteral("None"), Qt::CaseInsensitive) == 0)
        parity = QSerialPort::NoParity;
    else if (m_parity.compare(QStringLiteral("Odd"), Qt::CaseInsensitive) == 0)
        parity = QSerialPort::OddParity;

    QSerialPort::DataBits dataBits = QSerialPort::Data8;
    if (m_dataBits == 7)
        dataBits = QSerialPort::Data7;

    const QSerialPort::StopBits stopBits = m_stopBits == 2 ? QSerialPort::TwoStop
                                                           : QSerialPort::OneStop;

    m_client->setConnectionParameter(QModbusDevice::SerialPortNameParameter, m_portName);
    m_client->setConnectionParameter(QModbusDevice::SerialBaudRateParameter, m_baudRate);
    m_client->setConnectionParameter(QModbusDevice::SerialParityParameter, parity);
    m_client->setConnectionParameter(QModbusDevice::SerialDataBitsParameter, dataBits);
    m_client->setConnectionParameter(QModbusDevice::SerialStopBitsParameter, stopBits);

    // Qt 仅负责单次请求超时；重试次数由当前通信工作对象统一管理。
    m_client->setTimeout(m_requestTimeoutMs);
    m_client->setNumberOfRetries(0);
}

void ModbusManager::attemptConnect()
{
    if (m_stopping || !m_client)
        return;
    if (m_client->state() == QModbusDevice::ConnectedState
        || m_client->state() == QModbusDevice::ConnectingState) {
        return;
    }

    emit connectionStateChanged(ConnectionState::Connecting,
                                QStringLiteral("Opening %1").arg(m_portName));
    // 为本次连接启用断线处理，串口打开失败同样进入该流程。
    m_disconnectHandled = false;
    if (!m_client->connectDevice())
        scheduleReconnect();//调度连接
}

//调度连接
void ModbusManager::scheduleReconnect()
{
    if (m_stopping || !m_reconnectTimer || m_reconnectTimer->isActive())
        return;

    const int delay = reconnectDelayForAttempt(m_reconnectAttempt);
    if (m_reconnectAttempt < 3)
        ++m_reconnectAttempt;
    m_reconnectTimer->start(delay);
}

void ModbusManager::handleConnected()
{
    m_disconnectHandled = false;
    m_reconnectAttempt = 0;//重新连接尝试
    if (m_reconnectTimer)//重新连接定时器
        m_reconnectTimer->stop();

    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    for (RuntimeDevice &device : m_devices)
        device.nextPollAtMs = now;

    emit connectionStateChanged(ConnectionState::Connected,
                                QStringLiteral("Connected to %1").arg(m_portName));
    handlePolling();
}

void ModbusManager::handleTransportDisconnected(const QString &detail)
{
    if (m_disconnectHandled)
        return;
    m_disconnectHandled = true;

    // 延迟执行控制命令存在风险；断线时写请求直接失败，重连后重新建立读轮询。
    if (m_hasCurrentRequest) {
        if (m_currentRequest.type == ModbusRequestType::Write)
            completeWriteRequest(m_currentRequest.requestId, false, detail);
        else {
            handleReadFailure(m_currentRequest);
            emit requestFailed(m_currentRequest.requestId, detail);
        }
    }
    clearCurrentReply();
    m_hasCurrentRequest = false;

    const QList<ModbusRequest> pending = m_requestQueue.takeAll();
    for (const ModbusRequest &request : pending) {
        if (request.type == ModbusRequestType::Write)
            completeWriteRequest(request.requestId, false, detail);
    }

    for (RuntimeDevice &device : m_devices) {
        if (device.statistics.state != DeviceState::Offline) {
            device.statistics.state = DeviceState::Offline;
            emit slaveStateChanged(device.statistics.slaveId, DeviceState::Offline, detail);
        }
    }
    for (int index = 0; index < static_cast<int>(MonitorPoint::Count); ++index)
        m_snapshot.markStale(static_cast<MonitorPoint>(index));
    emitSnapshot();

    if (!m_stopping)
        scheduleReconnect();
}

void ModbusManager::handlePolling()
{
    if (m_stopping || !m_client || m_client->state() != QModbusDevice::ConnectedState)
        return;

    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    for (auto iterator = m_devices.begin(); iterator != m_devices.end(); ++iterator) {
        RuntimeDevice &device = iterator.value();
        if (device.nextPollAtMs > now)
            continue;

        for (const ReadBlock &block : m_readBlocks) {
            if (block.slaveId == iterator.key())
                enqueueReadBlock(block);
        }
        device.nextPollAtMs = now + pollIntervalForState(device.statistics.state);
    }
    sendNextRequest();
}

void ModbusManager::enqueueReadBlock(const ReadBlock &block)
{
    ModbusRequest request;
    request.type = ModbusRequestType::Read;
    request.slaveId = block.slaveId;
    request.registerType = block.registerType;
    request.startAddress = block.startAddress;
    request.quantity = static_cast<quint16>(block.quantity);
    request.requestId = m_nextRequestId++;

    // 旧请求尚未完成时轮询定时器可能再次触发，因此需要避免重复读请求堆积。
    if (m_hasCurrentRequest && m_currentRequest.key() == request.key())
        return;
    m_requestQueue.enqueue(request);
}

void ModbusManager::enqueueControlWrite(const QString &controlName,
                                        quint16 value,
                                        quint64 requestId)
{
    const auto control = m_controls.constFind(controlName);
    if (control == m_controls.constEnd()) {
        emit writeRequestFinished(requestId, false,
                                  QStringLiteral("Unknown control: %1").arg(controlName));
        emit requestFailed(requestId, QStringLiteral("Unknown control: %1").arg(controlName));
        return;
    }

    ModbusRequest request;
    request.type = ModbusRequestType::Write;
    request.slaveId = control->slaveId;
    request.registerType = control->registerType;
    request.startAddress = control->address;
    request.quantity = 1;
    request.values = {value};
    request.requestId = requestId;

    // 相同的在途写请求不可中断，将新调用方记录为该请求的结果别名。
    if (m_hasCurrentRequest && m_currentRequest.key() == request.key()
        && m_currentRequest.values == request.values) {
        m_requestAliases[m_currentRequest.requestId].append(requestId);
        return;
    }

    const ModbusRequestQueue::Result result = m_requestQueue.enqueue(request);
    if (result.type == ModbusRequestQueue::ResultType::Duplicate) {
        m_requestAliases[result.effectiveRequestId].append(requestId);
    } else if (result.type == ModbusRequestQueue::ResultType::Replaced) {
        completeWriteRequest(result.replacedRequestId, false,
                             QStringLiteral("Superseded by a newer value"));
    }
    sendNextRequest();
}

void ModbusManager::sendNextRequest()
{
    if (m_stopping || m_hasCurrentRequest || m_requestQueue.isEmpty() || !m_client
        || m_client->state() != QModbusDevice::ConnectedState) {
        return;
    }

    m_currentRequest = m_requestQueue.dequeue();
    m_hasCurrentRequest = true;

    QModbusDataUnit unit(m_currentRequest.registerType,
                        m_currentRequest.startAddress,
                        m_currentRequest.quantity);
    if (m_currentRequest.type == ModbusRequestType::Read) {
        m_currentReply = m_client->sendReadRequest(unit, m_currentRequest.slaveId);
    } else {
        unit.setValues(m_currentRequest.values);
        m_currentReply = m_client->sendWriteRequest(unit, m_currentRequest.slaveId);
    }

    if (!m_currentReply) {
        finishCurrentRequest(QModbusDevice::UnknownError, m_client->errorString());
        return;
    }

    connect(m_currentReply, &QModbusReply::finished,
            this, &ModbusManager::handleReplyFinished);
    if (m_currentReply->isFinished())
        QTimer::singleShot(0, this, &ModbusManager::handleReplyFinished);
}

void ModbusManager::handleReplyFinished()
{
    QModbusReply *reply = qobject_cast<QModbusReply *>(sender());
    if (!reply)
        reply = m_currentReply;
    if (!m_hasCurrentRequest || !reply || reply != m_currentReply)
        return;

    const qint64 replyFinishedAtMs = QDateTime::currentMSecsSinceEpoch();
    const QModbusDevice::Error error = reply->error();
    const QString detail = reply->errorString();
    if (error != QModbusDevice::NoError) {
        finishCurrentRequest(error, detail);
        return;
    }

    const ModbusRequest completed = m_currentRequest;
    if (completed.type == ModbusRequestType::Read
        && reply->result().valueCount() < completed.quantity) {
        finishCurrentRequest(QModbusDevice::UnknownError,
                             QStringLiteral("Incomplete Modbus response"));
        return;
    }
    m_snapshot.responseTimestampMs = replyFinishedAtMs;
    updateDeviceSuccess(completed.slaveId);
    if (completed.type == ModbusRequestType::Read)
        handleReadSuccess(completed, reply->result());
    else
        completeWriteRequest(completed.requestId, true, QStringLiteral("Write completed"));

    clearCurrentReply();
    m_hasCurrentRequest = false;
    sendNextRequest();
}

void ModbusManager::finishCurrentRequest(QModbusDevice::Error error, const QString &detail)
{
    if (!m_hasCurrentRequest)
        return;

    const bool timedOut = error == QModbusDevice::TimeoutError;
    updateDeviceFailure(m_currentRequest.slaveId, timedOut, detail);

    // 请求一旦超时就立即将采样标记为陈旧，即使后续仍会重试该逻辑请求。
    if (timedOut && m_currentRequest.type == ModbusRequestType::Read)
        handleReadFailure(m_currentRequest);

    if (timedOut && shouldRetry(m_currentRequest.retryCount, m_maxRetries)) {
        // 将同一逻辑请求重新插入队首，每次重试都使用新的 Qt 单次超时周期。
        ModbusRequest retry = m_currentRequest;
        ++retry.retryCount;
        clearCurrentReply();
        m_hasCurrentRequest = false;
        m_requestQueue.prependRetry(retry);
        sendNextRequest();
        return;
    }

    const ModbusRequest failed = m_currentRequest;
    if (failed.type == ModbusRequestType::Read) {
        if (!timedOut)
            handleReadFailure(failed);
        emit requestFailed(failed.requestId, detail);
    } else {
        completeWriteRequest(failed.requestId, false, detail);
    }

    clearCurrentReply();
    m_hasCurrentRequest = false;

    if (error == QModbusDevice::ConnectionError && m_client
        && m_client->state() != QModbusDevice::UnconnectedState) {
        m_client->disconnectDevice();
        return;
    }
    sendNextRequest();
}

void ModbusManager::clearCurrentReply()
{
    if (!m_currentReply)
        return;
    disconnect(m_currentReply, nullptr, this, nullptr);
    m_currentReply->deleteLater();
    m_currentReply = nullptr;
}

void ModbusManager::handleReadSuccess(const ModbusRequest &request,
                                      const QModbusDataUnit &result)
{
    const QDateTime now = QDateTime::currentDateTime();
    for (const PointDefinition &point : m_points) {
        if (point.slaveId != request.slaveId || point.registerType != request.registerType
            || point.address < request.startAddress
            || point.address + point.registerCount > request.startAddress + request.quantity) {
            continue;
        }

        QVector<quint16> registers;
        const int offset = point.address - request.startAddress;
        for (int index = 0; index < point.registerCount; ++index)
            registers.append(result.value(offset + index));

        bool decoded = false;
        const double value = decodeValue(registers, point.dataType, point.divisor,
                                         point.byteOrder, &decoded);
        if (decoded)
            m_snapshot.setGood(point.point, value, now);
    }
    m_snapshot.timestamp = now;
    emitSnapshot();
}

void ModbusManager::handleReadFailure(const ModbusRequest &request)
{
    for (const PointDefinition &point : m_points) {
        if (point.slaveId == request.slaveId && point.registerType == request.registerType
            && point.address >= request.startAddress
            && point.address + point.registerCount <= request.startAddress + request.quantity) {
            m_snapshot.markStale(point.point);
        }
    }
    m_snapshot.timestamp = QDateTime::currentDateTime();
    m_snapshot.responseTimestampMs = QDateTime::currentMSecsSinceEpoch();
    emitSnapshot();
}

void ModbusManager::completeWriteRequest(quint64 requestId,
                                         bool success,
                                         const QString &detail)
{
    QList<quint64> requestIds = m_requestAliases.take(requestId);
    requestIds.prepend(requestId);
    for (quint64 id : requestIds) {
        emit writeRequestFinished(id, success, detail);
        if (!success)
            emit requestFailed(id, detail);
    }
}

void ModbusManager::updateDeviceSuccess(int slaveId)
{
    RuntimeDevice &device = m_devices[slaveId];
    const DeviceState oldState = device.statistics.state;
    device.statistics.slaveId = slaveId;
    ++device.statistics.successCount;
    device.statistics.consecutiveTimeouts = 0;
    device.statistics.lastSuccessTime = QDateTime::currentDateTime();
    device.statistics.state = DeviceState::Online;
    if (oldState != DeviceState::Online) {
        device.nextPollAtMs = QDateTime::currentMSecsSinceEpoch() + m_pollIntervalMs;
        emit slaveStateChanged(slaveId, DeviceState::Online, QStringLiteral("Device recovered"));
    }
}

void ModbusManager::updateDeviceFailure(int slaveId,
                                        bool timeout,
                                        const QString &detail)
{
    RuntimeDevice &device = m_devices[slaveId];
    device.statistics.slaveId = slaveId;
    ++device.statistics.errorCount;
    if (!timeout)
        return;

    const DeviceState oldState = device.statistics.state;
    ++device.statistics.consecutiveTimeouts;
    device.statistics.state = stateForConsecutiveTimeouts(
        device.statistics.consecutiveTimeouts);
    if (oldState != device.statistics.state) {
        device.nextPollAtMs = QDateTime::currentMSecsSinceEpoch()
                              + pollIntervalForState(device.statistics.state);
        emit slaveStateChanged(slaveId, device.statistics.state,
                               QStringLiteral("%1: %2")
                                   .arg(deviceStateText(device.statistics.state), detail));
    }
}

void ModbusManager::emitSnapshot()
{
    m_snapshot.devices.clear();
    for (const RuntimeDevice &device : m_devices)
        m_snapshot.devices.append(device.statistics);
    emit snapshotReady(m_snapshot);
}

int ModbusManager::pollIntervalForState(DeviceState state) const
{
    switch (state) {
    case DeviceState::Online:
        return m_pollIntervalMs;
    case DeviceState::Unstable:
        return 3000;
    case DeviceState::Error:
        return 10000;
    case DeviceState::Offline:
        return 30000;
    }
    return m_pollIntervalMs;
}

bool ModbusManager::pointFromName(const QString &name, MonitorPoint *point)
{
    const QString normalized = name.trimmed().toLower();
    static const QStringList names = {
        QStringLiteral("temp"), QStringLiteral("humidity"), QStringLiteral("pressure"),
        QStringLiteral("voc"), QStringLiteral("light"), QStringLiteral("ph"),
        QStringLiteral("tds"), QStringLiteral("ec"), QStringLiteral("turbidity"),
        QStringLiteral("temp1"), QStringLiteral("temp2"), QStringLiteral("waterlevel1"),
        QStringLiteral("waterlevel2")
    };
    const int index = names.indexOf(normalized);
    if (index < 0)
        return false;
    if (point)
        *point = static_cast<MonitorPoint>(index);
    return true;
}

QModbusDataUnit::RegisterType ModbusManager::registerTypeFromName(const QString &name,
                                                                  bool *ok)
{
    const QString normalized = name.trimmed().toLower();
    if (ok)
        *ok = true;
    if (normalized == QStringLiteral("coils"))
        return QModbusDataUnit::Coils;
    if (normalized == QStringLiteral("discreteinputs"))
        return QModbusDataUnit::DiscreteInputs;
    if (normalized == QStringLiteral("inputregisters"))
        return QModbusDataUnit::InputRegisters;
    if (normalized == QStringLiteral("holdingregisters"))
        return QModbusDataUnit::HoldingRegisters;
    if (ok)
        *ok = false;
    return QModbusDataUnit::Invalid;
}
