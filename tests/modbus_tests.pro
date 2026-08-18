QT += core testlib serialbus serialport
CONFIG += testcase console c++11
TEMPLATE = app
TARGET = modbus_tests

INCLUDEPATH += ..

SOURCES += \
    test_modbus.cpp \
    ../modbusmanger.cpp \
    ../modbusrequestqueue.cpp

HEADERS += \
    ../modbusmanger.h \
    ../modbusrequestqueue.h \
    ../monitor_types.h
