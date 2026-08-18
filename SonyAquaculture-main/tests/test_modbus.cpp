#include "modbusmanger.h"
#include "modbusrequestqueue.h"
#include "monitor_types.h"

#include <QSignalSpy>
#include <QFileInfo>
#include <QTemporaryFile>
#include <QTest>

class ModbusTests : public QObject
{
    Q_OBJECT

private slots:
    void readRequestsRemainFifo();
    void writesPreemptPendingReads();
    void duplicateReadIsRejected();
    void latestWriteReplacesQueuedValue();
    void identicalWriteReusesQueuedRequest();
    void decodesConfiguredRegisterFormats();
    void rejectsInvalidDecoding();
    void followsDeviceStateThresholds();
    void followsReconnectSchedule();
    void allowsInitialAttemptPlusThreeRetries();
    void marksGoodDataStaleWithoutLosingValue();
    void acceptsDemoConfiguration();
    void rejectsIncompletePointConfiguration();
};

namespace {

ModbusRequest readRequest(quint64 id, int address)
{
    ModbusRequest request;
    request.type = ModbusRequestType::Read;
    request.requestId = id;
    request.startAddress = address;
    request.quantity = 1;
    return request;
}

ModbusRequest writeRequest(quint64 id, int address, quint16 value)
{
    ModbusRequest request;
    request.type = ModbusRequestType::Write;
    request.requestId = id;
    request.startAddress = address;
    request.quantity = 1;
    request.values = {value};
    return request;
}

} // namespace

void ModbusTests::readRequestsRemainFifo()
{
    ModbusRequestQueue queue;
    queue.enqueue(readRequest(1, 0));
    queue.enqueue(readRequest(2, 1));
    QCOMPARE(queue.dequeue().requestId, quint64(1));
    QCOMPARE(queue.dequeue().requestId, quint64(2));
}

void ModbusTests::writesPreemptPendingReads()
{
    ModbusRequestQueue queue;
    queue.enqueue(readRequest(1, 0));
    queue.enqueue(readRequest(2, 1));
    queue.enqueue(writeRequest(3, 100, 7));
    QCOMPARE(queue.dequeue().requestId, quint64(3));
    QCOMPARE(queue.dequeue().requestId, quint64(1));
}

void ModbusTests::duplicateReadIsRejected()
{
    ModbusRequestQueue queue;
    queue.enqueue(readRequest(1, 0));
    const ModbusRequestQueue::Result result = queue.enqueue(readRequest(2, 0));
    QCOMPARE(result.type, ModbusRequestQueue::ResultType::Duplicate);
    QCOMPARE(result.effectiveRequestId, quint64(1));
    QCOMPARE(queue.size(), 1);
}

void ModbusTests::latestWriteReplacesQueuedValue()
{
    ModbusRequestQueue queue;
    queue.enqueue(writeRequest(1, 100, 1));
    const ModbusRequestQueue::Result result = queue.enqueue(writeRequest(2, 100, 9));
    QCOMPARE(result.type, ModbusRequestQueue::ResultType::Replaced);
    QCOMPARE(result.replacedRequestId, quint64(1));
    QCOMPARE(queue.dequeue().values.first(), quint16(9));
}

void ModbusTests::identicalWriteReusesQueuedRequest()
{
    ModbusRequestQueue queue;
    queue.enqueue(writeRequest(1, 100, 9));
    const ModbusRequestQueue::Result result = queue.enqueue(writeRequest(2, 100, 9));
    QCOMPARE(result.type, ModbusRequestQueue::ResultType::Duplicate);
    QCOMPARE(result.effectiveRequestId, quint64(1));
    QCOMPARE(queue.size(), 1);
}

void ModbusTests::decodesConfiguredRegisterFormats()
{
    bool ok = false;
    QCOMPARE(ModbusManager::decodeValue({250}, "UInt16", 10, "AB", &ok), 25.0);
    QVERIFY(ok);
    QCOMPARE(ModbusManager::decodeValue({0xff9c}, "Int16", 10, "AB", &ok), -10.0);
    QCOMPARE(ModbusManager::decodeValue({0x0001, 0x0002}, "UInt32", 1, "ABCD", &ok), 65538.0);
    QCOMPARE(ModbusManager::decodeValue({0x0002, 0x0001}, "UInt32", 1, "CDAB", &ok), 65538.0);
    QCOMPARE(ModbusManager::decodeValue({0x3f80, 0x0000}, "Float32", 1, "ABCD", &ok), 1.0);
}

void ModbusTests::rejectsInvalidDecoding()
{
    bool ok = true;
    ModbusManager::decodeValue({1}, "Float64", 1, "ABCD", &ok);
    QVERIFY(!ok);
    ModbusManager::decodeValue({1}, "UInt16", 0, "AB", &ok);
    QVERIFY(!ok);
}

void ModbusTests::followsDeviceStateThresholds()
{
    QCOMPARE(ModbusManager::stateForConsecutiveTimeouts(0), DeviceState::Online);
    QCOMPARE(ModbusManager::stateForConsecutiveTimeouts(3), DeviceState::Unstable);
    QCOMPARE(ModbusManager::stateForConsecutiveTimeouts(5), DeviceState::Error);
    QCOMPARE(ModbusManager::stateForConsecutiveTimeouts(10), DeviceState::Offline);
}

void ModbusTests::followsReconnectSchedule()
{
    QCOMPARE(ModbusManager::reconnectDelayForAttempt(0), 1000);
    QCOMPARE(ModbusManager::reconnectDelayForAttempt(1), 2000);
    QCOMPARE(ModbusManager::reconnectDelayForAttempt(2), 4000);
    QCOMPARE(ModbusManager::reconnectDelayForAttempt(3), 30000);
    QCOMPARE(ModbusManager::reconnectDelayForAttempt(8), 30000);
}

void ModbusTests::allowsInitialAttemptPlusThreeRetries()
{
    QVERIFY(ModbusManager::shouldRetry(0, 3));
    QVERIFY(ModbusManager::shouldRetry(1, 3));
    QVERIFY(ModbusManager::shouldRetry(2, 3));
    QVERIFY(!ModbusManager::shouldRetry(3, 3));
}

void ModbusTests::marksGoodDataStaleWithoutLosingValue()
{
    MonitorSnapshot snapshot;
    snapshot.setGood(MonitorPoint::Temp, 25.5, QDateTime::currentDateTime());
    snapshot.markStale(MonitorPoint::Temp);
    QCOMPARE(snapshot.quality(MonitorPoint::Temp), DataQuality::Stale);
    QCOMPARE(snapshot.value(MonitorPoint::Temp), 25.5);
    QCOMPARE(snapshot.quality(MonitorPoint::Humidity), DataQuality::Bad);
}

void ModbusTests::acceptsDemoConfiguration()
{
    const QString configPath = QFileInfo(QString::fromUtf8(__FILE__))
                                   .dir().filePath(QStringLiteral("../modbus.ini"));
    QVERIFY2(QFileInfo::exists(configPath), qPrintable(configPath));

    ModbusManager manager(configPath);
    QSignalSpy errorSpy(&manager, &ModbusManager::configurationError);
    manager.start();
    const QString error = errorSpy.isEmpty() ? QString() : errorSpy.first().first().toString();
    QVERIFY2(errorSpy.isEmpty(), qPrintable(error));
    manager.stop();
}

void ModbusTests::rejectsIncompletePointConfiguration()
{
    QTemporaryFile file;
    QVERIFY(file.open());
    file.write("[Serial]\nport=COM1\n[Polling]\npollIntervalMs=1000\n");
    file.flush();

    ModbusManager manager(file.fileName());
    QSignalSpy errorSpy(&manager, &ModbusManager::configurationError);
    manager.start();
    QCOMPARE(errorSpy.count(), 1);
}

QTEST_MAIN(ModbusTests)
#include "test_modbus.moc"
