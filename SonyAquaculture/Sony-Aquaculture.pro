#-------------------------------------------------
#
# Project created by QtCreator 2023-11-09T14:11:35
#
#-------------------------------------------------
QT       += core gui charts sql serialbus serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Sony-Aquaculture

TEMPLATE = app

RC_ICONS = logo.ico

DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += c++11 #console

SOURCES += \
        faultdetect.cpp \
        globalmacro.cpp \
        log.cpp \
        main.cpp \
        mainwindow.cpp \
        menu.cpp \
        modbusmanger.cpp \
        modbusrequestqueue.cpp \
        mypoint.cpp \
        realtimegraphdata.cpp \
        systemstate.cpp \
        waterbottom.cpp \
        watertop.cpp

HEADERS += \
        faultdetect.h \
        globalmacro.h \
        log.h \
        mainwindow.h \
        menu.h \
        modbusmanger.h \
        modbusrequestqueue.h \
        monitor_types.h \
        mypoint.h \
        realtimegraphdata.h \
        systemstate.h \
        waterbottom.h \
        watertop.h

FORMS += \
        faultdetect.ui \
        log.ui \
        mainwindow.ui \
        menu.ui \
        mypoint.ui \
        systemstate.ui \
        waterbottom.ui \
        watertop.ui

#指定可执行文件目录 放到这里省了拷贝动态库动作 专为小白懒人考虑
DESTDIR     = $$PWD/exe
#引入库头文件目录
INCLUDEPATH += $$PWD/include
#加载库
LIBS+=  -L$$PWD/lib -lslideButtonLib

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc

DISTFILES += \
    modbus.ini
