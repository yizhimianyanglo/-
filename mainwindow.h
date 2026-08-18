#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>//关闭窗口事件头文件
#include"menu.h"//菜单类
#include "watertop.h"//水上数据类
#include "waterbottom.h"//水下数据类
#include "systemstate.h"//系统状态类
#include "log.h"//日志类
#include<QChart>//视图类
#include <QHash>
#include <QSqlQueryModel>
#include "monitor_types.h"
#include "realtimegraphdata.h"
#include <functional>
#include "slidebuttonlib.h"
#include "slidebuttonlib_global.h"
#include "faultdetect.h"
#define WARN 1
#define PROMPT 2
using namespace QtCharts;
namespace Ui {
class MainWindow;
}

class ModbusManager;
class QThread;
class QTimer;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
protected:
    void init();
    //---------- 窗口移动事件(重写) ----------
    //鼠标按下事件
    void mousePressEvent(QMouseEvent *event);
    //鼠标释放事件
    void mouseReleaseEvent(QMouseEvent *event);
    //鼠标移动事件
    void mouseMoveEvent(QMouseEvent *event);
    //关闭窗口事件
    void closeEvent(QCloseEvent *event);
    //DockWidget窗口初始化
    void DockWidgetInit();
    //RealTimePage页面初始化
    void RealTimePageInit();
    //RecordQueryPage页面初始化
    void RecordQueryPageInit();
    //RecordQueryPage页面----插入记录
    void appendLog(int msgtype,QString msg);
    //ControlPage页面初始化
    void ControlPageInit();
    //FaultDetect页面初始化
    void FaultDetectInit();
    void ModbusInit();
    void DatabaseRuntimeInit();
    void handleSnapshot(const MonitorSnapshot &snapshot);
    quint64 submitControlWrite(const QString &controlName, quint16 value,
                               const std::function<void()> &onSuccess);
    void storeLatestSnapshot();
    void updateConnectionIndicators();
private slots:
    //RealTimePage页面----ChartComboBox切换数据视图
    void on_ChartComboBox_currentIndexChanged(const QString &arg1);

signals:
    void startModbus();
    void stopModbus();
    void controlWriteRequested(const QString &controlName, quint16 value, quint64 requestId);
private:
    Ui::MainWindow *ui;
    bool m_bIsPressed=false;	//鼠标按下标志位
    QPoint m_lastPt;	//记录第一次鼠标按下的局部坐标
    Menu *menu;//菜单-窗口
    WaterTop *waterTop;//水上数据-窗口
    WaterBottom *waterBottom;//水下数据-窗口
    SystemState *systemState;//系统状态-窗口
    Log *log;//日志-窗口
    QSqlQueryModel* qmodelRecoed = nullptr;
    //------曲线图----------
    QChart *chart = nullptr;
    GraphData *graphData = nullptr;
    //------通信类----------
    ModbusManager *m_modbusManager = nullptr;
    QThread *m_modbusThread = nullptr;
    QTimer *m_storageTimer = nullptr;
    QTimer *m_databaseReconnectTimer = nullptr;
    MonitorSnapshot m_latestSnapshot;
    bool m_hasSnapshot = false;
    bool m_modbusOnline = false;
    bool m_databaseOnline = false;
    quint64 m_nextControlRequestId = 1;
    QHash<quint64, std::function<void()>> m_pendingControlUpdates;
    //------子线程类---------

    //------模块状态类-------
    FaultDetect *myFaultDetect = nullptr;



};

#endif // MAINWINDOW_H
