#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "modbusmanger.h"
#include<QComboBox>
#include<QDateTime>
#include<QDebug>
#include<QDockWidget>
#include<QLabel>
#include<QListView>
#include<QSqlQuery>
#include<QSqlDatabase>
#include<QSqlError>
#include<QSettings>
#include<QThread>
#include<QTimer>
#include<QDir>
#include<QFileInfo>
#include<QCoreApplication>
#include<QtGlobal>
#include<QScrollBar>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    init();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::init()
{   
    setWindowFlags(Qt::FramelessWindowHint);//设置无系统边框  //Qt::WindowStaysOnTopHint窗口最高权限-窗口始终在最上层
    //this->setAttribute(Qt::WA_TranslucentBackground, true);//设置透明-窗体标题栏不透明,背景透明
    setGeometry(110,30,geometry().width(),geometry().height());//窗口初始显示位置


    DockWidgetInit();//DockWidget窗口初始化

    RealTimePageInit();//RealTimePage页面初始化

    RecordQueryPageInit();//RecordQueryPage页面初始化

    ControlPageInit();//ControlPage页面初始化

    FaultDetectInit();//FaultDetectPage页面初始化

    ui->stackedWidget->setCurrentIndex(0);
    DatabaseRuntimeInit();
    ModbusInit();
}

//---------- 窗口移动事件(重写) ----------
//鼠标按下事件
void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_bIsPressed = true;
        m_lastPt = event->globalPos() - this->pos();
        event->accept();
    }
}
//鼠标释放事件
void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    m_bIsPressed = false;	//鼠标按下标志位还原置为false
    Q_UNUSED(event);
}
//鼠标移动事件
void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (m_bIsPressed && (event->buttons() & Qt::LeftButton))
    {
        move(event->globalPos() - m_lastPt);
        event->accept();
    }
}
//关闭窗口事件
void MainWindow::closeEvent(QCloseEvent *event)
{
    // Stop serial I/O in its owning thread before the event loop is terminated.
    if (m_modbusManager && m_modbusThread && m_modbusThread->isRunning()) {
        QMetaObject::invokeMethod(m_modbusManager, "stop", Qt::BlockingQueuedConnection);
        m_modbusThread->quit();
        m_modbusThread->wait(5000);
    }
    event->accept();
}

//DockWidget窗口初始化
void MainWindow::DockWidgetInit()
{
   // QDockWidget 是 Qt 中的一个非常有用的类，它允许你创建可以停靠（dock）在主窗口不同位置的面板。这些面板可以被用户拖动、浮动显示或者停靠在主窗口的边缘。
    //--------------------------- Dock_Top-菜单栏 ----------------------------
    {
       QDockWidget *Dock_Top=new QDockWidget(this);
        //隐藏dw_top标题栏
        QWidget* pTitleWidget = Dock_Top ->titleBarWidget();
        QWidget* pWidget = new QWidget;
        Dock_Top ->setTitleBarWidget(pWidget); // 设置一个空的标题栏 替代了默认的标题栏，但自己不显示任何东西
        delete pTitleWidget ;
        //实例化菜单界面
        menu=new Menu(this);
        Dock_Top->setWidget(menu);
        Dock_Top->setFeatures(QDockWidget::NoDockWidgetFeatures);//设置不可移动、停靠
        addDockWidget(Qt::TopDockWidgetArea, Dock_Top);//设置在顶部
        //菜单界面信号的绑定
        connect(menu,&Menu::showMin,this,&MainWindow::showMinimized);//最小化
        connect(menu,&Menu::showMax,this,&MainWindow::showMaximized);//最大化
        connect(menu,&Menu::showNormal,this,&MainWindow::showNormal);//还原
        connect(menu,&Menu::showClose,this,&MainWindow::close);//关闭
        connect(menu,&Menu::toRealTimePage,this,[=]{ui->stackedWidget->setCurrentIndex(0);});
        connect(menu,&Menu::toRecordQueryPage,this,[=]{ui->stackedWidget->setCurrentIndex(1);});
        connect(menu,&Menu::toControlPage,this,[=]{ui->stackedWidget->setCurrentIndex(2);});
        connect(menu,&Menu::toFaultDetectPage,this,[=]{ui->stackedWidget->setCurrentIndex(3);});
    }
    QString LabelStyleSheet="background:rgb(36,61,91);color: rgb(122, 175, 227);font:14pt '楷体';font-weight:bold";
    QString DockStyleSheet="border:1px solid rgb(19,39,67);";
    //--------------------------- Dock_Left-水上数据 ---------------------------
    {
        QDockWidget *Dock_Left_Top=new QDockWidget("水上数据");
        Dock_Left_Top->setStyleSheet(DockStyleSheet);
        QLabel *Dock_Left_Top_Title=new QLabel;
        Dock_Left_Top_Title->setStyleSheet(LabelStyleSheet);
        Dock_Left_Top_Title->setText("水上数据");
        Dock_Left_Top_Title->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
        Dock_Left_Top->setTitleBarWidget(Dock_Left_Top_Title);
        waterTop=new WaterTop;
        Dock_Left_Top->setWidget(waterTop);
        //   停靠区域 对象 停靠方向
        addDockWidget(Qt::LeftDockWidgetArea, Dock_Left_Top,Qt::Orientation::Vertical);//设置在左侧，第三个参数表示DockWidget的方向是垂直的
        connect(waterTop,&WaterTop::writeLog,this,[=](QString objname,int type){
            QString typeMsg;
            if(objname=="Temp")
            {
                if(type==WARN)
                {
                    typeMsg="警告";
                }
                else if (type==PROMPT)
                {
                    typeMsg="提示";
                }
                log->appendLog(QString("水上温度异常%1！").arg(typeMsg));
                appendLog(type,QString("水上温度异常%1！").arg(typeMsg));
            }
            else if(objname=="Humidity")
            {
                if(type==WARN)
                {
                    typeMsg="警告";
                }
                else if (type==PROMPT)
                {
                    typeMsg="提示";
                }
                log->appendLog(QString("水上湿度异常%1！").arg(typeMsg));
                appendLog(type,QString("水上湿度异常%1！").arg(typeMsg));
            }
            else if(objname=="Pressure")
            {
                if(type==WARN)
                {
                    typeMsg="警告";
                }
                else if (type==PROMPT)
                {
                    typeMsg="提示";
                }
                log->appendLog(QString("水上气压异常%1！").arg(typeMsg));
                appendLog(type,QString("水上气压异常%1！").arg(typeMsg));
            }
            else if(objname=="Voc")
            {
                if(type==WARN)
                {
                    typeMsg="警告";
                }
                else if (type==PROMPT)
                {
                    typeMsg="提示";
                }
                log->appendLog(QString("水上VOC异常%1！").arg(typeMsg));
                appendLog(type,QString("水上VOC异常%1！").arg(typeMsg));
            }
            else if(objname=="Light")
            {
                if(type==WARN)
                {
                    typeMsg="警告";
                }
                else if (type==PROMPT)
                {
                    typeMsg="提示";
                }
                log->appendLog(QString("水上光照强度异常%1！").arg(typeMsg));
                appendLog(type,QString("水上光照强度异常%1！").arg(typeMsg));
            }
        });
    }


    //--------------------------- Dock_Left-水下数据 ---------------------------
    {
        QDockWidget *Dock_Left_Bottom=new QDockWidget("水下数据");
        Dock_Left_Bottom->setStyleSheet(DockStyleSheet);
        QLabel *Dock_Left_Bottom_Title=new QLabel;
        Dock_Left_Bottom_Title->setStyleSheet(LabelStyleSheet);
        Dock_Left_Bottom_Title->setText("水下数据");
        Dock_Left_Bottom_Title->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
        Dock_Left_Bottom->setTitleBarWidget(Dock_Left_Bottom_Title);
        waterBottom=new WaterBottom;
        Dock_Left_Bottom->setWidget(waterBottom);
        addDockWidget(Qt::LeftDockWidgetArea, Dock_Left_Bottom,Qt::Orientation::Vertical);//设置在左侧，第三个参数表示DockWidget的方向是垂直的
        connect(waterBottom,&WaterBottom::writeLog,this,[=](QString objname,int type){
            QString typeMsg;
            if(objname=="PH")
            {
                if(type==WARN)
                {
                    typeMsg="警告";
                }
                else if (type==PROMPT)
                {
                    typeMsg="提示";
                }
                log->appendLog(QString("水下PH异常%1！").arg(typeMsg));
                appendLog(type,QString("水下PH异常%1！").arg(typeMsg));
            }
            else if(objname=="Tds")
            {
                if(type==WARN)
                {
                    typeMsg="警告";
                }
                else if (type==PROMPT)
                {
                    typeMsg="提示";
                }
                log->appendLog(QString("水下TDS异常%1！").arg(typeMsg));
                appendLog(type,QString("水下TDS异常%1！").arg(typeMsg));
            }
            else if(objname=="Ec")
            {
                if(type==WARN)
                {
                    typeMsg="警告";
                }
                else if (type==PROMPT)
                {
                    typeMsg="提示";
                }
                log->appendLog(QString("水下EC异常%1！").arg(typeMsg));
                appendLog(type,QString("水下EC异常%1！").arg(typeMsg));
            }
            else if(objname=="Turbidity")
            {
                if(type==WARN)
                {
                    typeMsg="警告";
                }
                else if (type==PROMPT)
                {
                    typeMsg="提示";
                }
                log->appendLog(QString("水下浊度异常%1！").arg(typeMsg));
                appendLog(type,QString("水下浊度异常%1！").arg(typeMsg));
            }
            else if(objname=="Temp1")
            {
                if(type==WARN)
                {
                    typeMsg="警告";
                }
                else if (type==PROMPT)
                {
                    typeMsg="提示";
                }
                log->appendLog(QString("水下温度1异常%1！").arg(typeMsg));
                appendLog(type,QString("水下温度1异常%1！").arg(typeMsg));
            }
            else if(objname=="Temp2")
            {
                if(type==WARN)
                {
                    typeMsg="警告";
                }
                else if (type==PROMPT)
                {
                    typeMsg="提示";
                }
                log->appendLog(QString("水下温度2异常%1！").arg(typeMsg));
                appendLog(type,QString("水下温度2异常%1！").arg(typeMsg));
            }
            else if(objname=="WaterLevel1")
            {
                if(type==WARN)
                {
                    typeMsg="警告";
                }
                else if (type==PROMPT)
                {
                    typeMsg="提示";
                }
                log->appendLog(QString("水下水位1异常%1！").arg(typeMsg));
                appendLog(type,QString("水下水位1异常%1！").arg(typeMsg));
            }
            else if(objname=="WaterLevel2")
            {
                if(type==WARN)
                {
                    typeMsg="警告";
                }
                else if (type==PROMPT)
                {
                    typeMsg="提示";
                }
                log->appendLog(QString("水下水位2异常%1！").arg(typeMsg));
                appendLog(type,QString("水下水位2异常%1！").arg(typeMsg));
            }
        });
    }

    //--------------------------- Dock_Right-系统状态 ---------------------------
    {
        QDockWidget *Dock_Right_Top=new QDockWidget("系统状态");
        Dock_Right_Top->setStyleSheet(DockStyleSheet);
        QLabel *Dock_Right_Top_Title=new QLabel;
        Dock_Right_Top_Title->setStyleSheet(LabelStyleSheet);
        Dock_Right_Top_Title->setText("系统状态");
        Dock_Right_Top_Title->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
        Dock_Right_Top->setTitleBarWidget(Dock_Right_Top_Title);
        systemState=new SystemState;
        Dock_Right_Top->setWidget(systemState);
        addDockWidget(Qt::RightDockWidgetArea, Dock_Right_Top,Qt::Orientation::Vertical);//设置在右，第三个参数表示DockWidget的方向是垂直的
    }

    //--------------------------- Dock_Left-日志 ---------------------------
    {
        QDockWidget *Dock_Right_Bottom=new QDockWidget("日志");
        Dock_Right_Bottom->setStyleSheet(DockStyleSheet);
        QLabel *Dock_Right_Bottom_Title=new QLabel;
        Dock_Right_Bottom_Title->setStyleSheet(LabelStyleSheet);
        Dock_Right_Bottom_Title->setText("日志");
        Dock_Right_Bottom_Title->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
        Dock_Right_Bottom->setTitleBarWidget(Dock_Right_Bottom_Title);
        log=new Log;
        Dock_Right_Bottom->setWidget(log);
        addDockWidget(Qt::RightDockWidgetArea, Dock_Right_Bottom,Qt::Orientation::Vertical);//设置在右，第三个参数表示DockWidget的方向是垂直的
    }

}

//RealTimePage页面初始化
void MainWindow::RealTimePageInit()
{
    graphData = new GraphData(this);
    chart = new QChart();
    graphData->setupChart(chart, ui->Chart, ui->ChartComboBox->currentText());
    ui->ChartComboBox->setView(new QListView());

}
//RealTimePage页面----ChartComboBox切换数据视图
void MainWindow::on_ChartComboBox_currentIndexChanged(const QString &arg1)
{
    if (graphData)
        graphData->setView(arg1);
}

//RecordQueryPage页面初始化
void MainWindow::RecordQueryPageInit()
{
    QString scrollBarStyle = "QScrollBar{ background: rgb(14, 26, 50); width: 12px; }"
                             "QScrollBar::handle { background: rgb(19, 39, 67); border: 0px solid rgb(255, 255, 255); border-radius: 5px; }"
                             "QScrollBar::handle:hover { background: rgb(122, 175, 229); }"
                             "QScrollBar::sub-line, QScrollBar::add-line { background: rgb(19, 48, 80); }"
                             "QScrollBar::sub-page:vertical, QScrollBar::add-page:vertical { background: rgb(19, 48, 80); }";
    ui->RecordTableView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui->RecordTableView->horizontalHeader()->setFixedHeight(25);//设置表头的固定高度为25像素。
    ui->RecordTableView->horizontalHeader()->setHighlightSections(false); //该属性表示是否高亮显示包含所选项目的部分
    ui->RecordTableView->verticalHeader()->setVisible(false);   //隐藏列表头
    ui->RecordTableView->setSelectionBehavior(QAbstractItemView::SelectItems);//可以选择单个的项
    ui->RecordTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);//用户不能直接在表格或列表的单元格中进行编辑
    ui->RecordTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);//设置大小与行内容匹配且鼠标不可拖拽
    ui->RecordTableView->horizontalHeader()->setStretchLastSection(true);
    ui->RecordTableView->verticalScrollBar()->setStyleSheet(scrollBarStyle);
    ui->RecordTableView->horizontalScrollBar()->setStyleSheet(scrollBarStyle);
    qmodelRecoed=new QSqlQueryModel;
    qmodelRecoed->setQuery("select *from Record;");
    qmodelRecoed->setHeaderData(0, Qt::Horizontal, tr("序号"));
    qmodelRecoed->setHeaderData(1, Qt::Horizontal, tr("时间"));
    qmodelRecoed->setHeaderData(2, Qt::Horizontal, tr("类型"));
    qmodelRecoed->setHeaderData(3, Qt::Horizontal, tr("消息"));
    ui->RecordTableView->setModel(qmodelRecoed);
    //设置下拉框视图
    ui->RQSreachBoxType->setView(new QListView());
    ui->RQSreachBoxTime->setView(new QListView());
    //查询按钮
    connect(ui->RQSreachBtn,&QToolButton::clicked,this,[=]{
        QString type=ui->RQSreachBoxType->currentText();
        QString time=ui->RQSreachBoxTime->currentText();
        if(type=="全部"&&time=="全部")
            qmodelRecoed->setQuery("select *from Record ORDER BY Time DESC;");
        else if(type=="全部"&&time=="今天")
            qmodelRecoed->setQuery("SELECT * FROM Record WHERE DATE(Time)=CURDATE() ORDER BY Time DESC;");
        else if(type=="全部"&&time=="最近三天")
            qmodelRecoed->setQuery("SELECT * FROM Record WHERE Time>=CURDATE()-INTERVAL 3 DAY ORDER BY Time DESC;");
        else if(type=="全部"&&time=="最近七天")
            qmodelRecoed->setQuery("SELECT * FROM Record WHERE Time>=CURDATE()-INTERVAL 7 DAY ORDER BY Time DESC;");
        else if(type=="全部"&&time=="最近一个月")
            qmodelRecoed->setQuery("SELECT * FROM Record WHERE Time>=CURDATE()-INTERVAL 1 MONTH ORDER BY Time DESC;");
        else if(type=="全部"&&time=="最近三个月")
            qmodelRecoed->setQuery("SELECT * FROM Record WHERE Time>=CURDATE()-INTERVAL 3 MONTH ORDER BY Time DESC;");
        else if(type=="全部"&&time=="最近半年")
            qmodelRecoed->setQuery("SELECT * FROM Record WHERE Time>=CURDATE()-INTERVAL 6 MONTH ORDER BY Time DESC;");
        else if(type=="全部"&&time=="最近一年")
            qmodelRecoed->setQuery("SELECT * FROM Record WHERE Time>=CURDATE()-INTERVAL 1 YEAR ORDER BY Time DESC;");
        else if(type!="全部"&&time=="全部")
            qmodelRecoed->setQuery(QString("select *from Record where Type='%1' ORDER BY Time DESC;").arg(type));
        else if(type!="全部"&&time=="今天")
            qmodelRecoed->setQuery(QString("SELECT * FROM Record WHERE DATE(Time)=CURDATE() and Type='%1' ORDER BY Time DESC;").arg(type));
        else if(type!="全部"&&time=="最近三天")
            qmodelRecoed->setQuery(QString("SELECT * FROM Record WHERE Time>=CURDATE()-INTERVAL 3 DAY and Type='%1' ORDER BY Time DESC;").arg(type));
        else if(type!="全部"&&time=="最近七天")
            qmodelRecoed->setQuery(QString("SELECT * FROM Record WHERE Time>=CURDATE()-INTERVAL 7 DAY and Type='%1' ORDER BY Time DESC;").arg(type));
        else if(type!="全部"&&time=="最近一个月")
            qmodelRecoed->setQuery(QString("SELECT * FROM Record WHERE Time>=CURDATE()-INTERVAL 1 MONTH and Type='%1' ORDER BY Time DESC;").arg(type));
        else if(type!="全部"&&time=="最近三个月")
            qmodelRecoed->setQuery(QString("SELECT * FROM Record WHERE Time>=CURDATE()-INTERVAL 3 MONTH and Type='%1' ORDER BY Time DESC;").arg(type));
        else if(type!="全部"&&time=="最近半年")
            qmodelRecoed->setQuery(QString("SELECT * FROM Record WHERE Time>=CURDATE()-INTERVAL 6 MONTH and Type='%1' ORDER BY Time DESC;").arg(type));
        else if(type!="全部"&&time=="最近一年")
            qmodelRecoed->setQuery(QString("SELECT * FROM Record WHERE Time>=CURDATE()-INTERVAL 1 YEAR and Type='%1' ORDER BY Time DESC;").arg(type));

        //调用 setModel() 后，QTableView 会立即从 qmodelRecoed 中读取数据，并重新绘制表格
        ui->RecordTableView->setModel(qmodelRecoed);
    });
}
//RecordQueryPage页面----插入记录
void MainWindow::appendLog(int msgtype,QString msg)
{
    QString type;
    if(msgtype==WARN)
    {type="警告";}
    else if (msgtype==PROMPT)
    {type="提示"; }
    QString nowTime=QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    int RowCont=qmodelRecoed->rowCount();
    QSqlQuery sql;
    QString sqlexec=QString("insert into Record values(%1,'%2','%3','%4');").arg(RowCont+1).arg(nowTime).arg(type).arg(msg);
    if(!sql.exec(sqlexec))
    {
        qDebug()<<"执行错误的SQL:\n"<<sqlexec<<endl;
        return;
    }
    else
    {
        qmodelRecoed->setQuery("select *from Record;");
        ui->RecordTableView->setModel(qmodelRecoed);
    }
}

//ControlPage页面初始化
void MainWindow::ControlPageInit()
{
    ui->ControlStackedWidget->setCurrentIndex(0);//默认显示投喂控制页面
    //==============视图切换按钮==============
    {
        QString defaultStyleSheet = "QToolButton{color: rgb(122, 175, 227);font: 10pt '楷体';border-radius:6px;}"
                                    "QToolButton:hover{background-color: rgb(19, 48, 80);font: 10pt '楷体';border-radius:6px;border:2px solid rgb(13,39,67);}";
        QString pressedStyleSheet = "QToolButton{background-color: rgb(3, 57, 103);font: 10pt '楷体';border-radius:8px;border:2px solid rgb(13,39,67);}";

        connect(ui->ControlBtnAutoTW,&QToolButton::clicked,this,[=]{
            ui->ControlStackedWidget->setCurrentIndex(0);//投喂控制
            ui->ControlBtnAutoTW->setStyleSheet(pressedStyleSheet);
            ui->ControlBtnAutoWD->setStyleSheet(defaultStyleSheet);
            ui->ControlBtnAutoSW->setStyleSheet(defaultStyleSheet);
            ui->ControlBtnAutoHJ->setStyleSheet(defaultStyleSheet);
            ui->ControlBtnAutoYQ->setStyleSheet(defaultStyleSheet);
            ui->ControlBtnAutoGD->setStyleSheet(defaultStyleSheet);
        });
        connect(ui->ControlBtnAutoWD,&QToolButton::clicked,this,[=]{
            ui->ControlStackedWidget->setCurrentIndex(1);//温度控制
            ui->ControlBtnAutoTW->setStyleSheet(defaultStyleSheet);
            ui->ControlBtnAutoWD->setStyleSheet(pressedStyleSheet);
            ui->ControlBtnAutoSW->setStyleSheet(defaultStyleSheet);
            ui->ControlBtnAutoHJ->setStyleSheet(defaultStyleSheet);
            ui->ControlBtnAutoYQ->setStyleSheet(defaultStyleSheet);
            ui->ControlBtnAutoGD->setStyleSheet(defaultStyleSheet);
        });
        connect(ui->ControlBtnAutoSW,&QToolButton::clicked,this,[=]{
            ui->ControlStackedWidget->setCurrentIndex(2);//水位控制
            ui->ControlBtnAutoTW->setStyleSheet(defaultStyleSheet);
            ui->ControlBtnAutoWD->setStyleSheet(defaultStyleSheet);
            ui->ControlBtnAutoSW->setStyleSheet(pressedStyleSheet);
            ui->ControlBtnAutoHJ->setStyleSheet(defaultStyleSheet);
            ui->ControlBtnAutoYQ->setStyleSheet(defaultStyleSheet);
            ui->ControlBtnAutoGD->setStyleSheet(defaultStyleSheet);
        });
        connect(ui->ControlBtnAutoHJ,&QToolButton::clicked,this,[=]{
            ui->ControlStackedWidget->setCurrentIndex(3);//环境控制
            ui->ControlBtnAutoTW->setStyleSheet(defaultStyleSheet);
            ui->ControlBtnAutoWD->setStyleSheet(defaultStyleSheet);
            ui->ControlBtnAutoSW->setStyleSheet(defaultStyleSheet);
            ui->ControlBtnAutoHJ->setStyleSheet(pressedStyleSheet);
            ui->ControlBtnAutoYQ->setStyleSheet(defaultStyleSheet);
            ui->ControlBtnAutoGD->setStyleSheet(defaultStyleSheet);
        });
        connect(ui->ControlBtnAutoYQ,&QToolButton::clicked,this,[=]{
            ui->ControlStackedWidget->setCurrentIndex(4);//氧气控制
            ui->ControlBtnAutoTW->setStyleSheet(defaultStyleSheet);
            ui->ControlBtnAutoWD->setStyleSheet(defaultStyleSheet);
            ui->ControlBtnAutoSW->setStyleSheet(defaultStyleSheet);
            ui->ControlBtnAutoHJ->setStyleSheet(defaultStyleSheet);
            ui->ControlBtnAutoYQ->setStyleSheet(pressedStyleSheet);
            ui->ControlBtnAutoGD->setStyleSheet(defaultStyleSheet);
        });
        connect(ui->ControlBtnAutoGD,&QToolButton::clicked,this,[=]{
            ui->ControlStackedWidget->setCurrentIndex(5);//供电控制
            ui->ControlBtnAutoTW->setStyleSheet(defaultStyleSheet);
            ui->ControlBtnAutoWD->setStyleSheet(defaultStyleSheet);
            ui->ControlBtnAutoSW->setStyleSheet(defaultStyleSheet);
            ui->ControlBtnAutoHJ->setStyleSheet(defaultStyleSheet);
            ui->ControlBtnAutoYQ->setStyleSheet(defaultStyleSheet);
            ui->ControlBtnAutoGD->setStyleSheet(pressedStyleSheet);
        });
    }
    //============== 投喂控制 ==============
    {
        ui->btn_FeedingAuto->setOnBgBrush(QColor(36,61,91));
        ui->btn_FeedingAuto->setOffBgBrush(QColor(36,61,91));
        //投喂是否开启按钮
        connect(ui->btn_FeedingAuto,&SlideButtonLib::isOffValueState,this,[=](bool isOff){
            submitControlWrite(QStringLiteral("FeedingAuto"), isOff ? 0 : 1, [this, isOff]() {
                ui->feedingAutoValue->setText(isOff ? QStringLiteral("OFF") : QStringLiteral("ON"));
                systemState->setStateFeeding(isOff ? QStringLiteral("未开启") : QStringLiteral("已开启"));
                myFaultDetect->setTWKZmodelType(isOff ? 0 : 1);
            });
        });
        //定次设置
        connect(ui->feedingNumBox_btn,&QPushButton::clicked,this,[=]{
            const int value = ui->feedingNumBox->currentIndex();
            const QString text = (value == 0 || value == 2) ? QStringLiteral("0/1")
                                                            : QStringLiteral("0/2");
            submitControlWrite(QStringLiteral("FeedingFrequency"), value,
                               [this, text]() { ui->feedingValue->setText(text); });
        });
        //定量设置
        connect(ui->feedingCountBox_btn,&QPushButton::clicked,this,[=]{
            const int value = ui->feedingCountBox->currentIndex();
            const QString text = ui->feedingCountBox->currentText().section(QStringLiteral("占比"), 1);
            submitControlWrite(QStringLiteral("FeedingAmount"), value,
                               [this, text]() { ui->feedingWeightValue->setText(text); });
        });
    }
    //============== 温度控制 ==============
    {
        ui->btn_TempAuto->setOnBgBrush(QColor(36,61,91));
        ui->btn_TempAuto->setOffBgBrush(QColor(36,61,91));
        //自动温度是否开启按钮
        connect(ui->btn_TempAuto,&SlideButtonLib::isOffValueState,this,[=](bool isOff){
            submitControlWrite(QStringLiteral("TempAuto"), isOff ? 0 : 1, [this, isOff]() {
                ui->tempAutoValue->setText(isOff ? QStringLiteral("OFF") : QStringLiteral("ON"));
                systemState->setStateTemp(isOff ? QStringLiteral("未开启") : QStringLiteral("已开启"));
            });
        });
    }
    //============== 水位控制 ==============
    {
        ui->btn_WaterlevelAuto->setOnBgBrush(QColor(36,61,91));
        ui->btn_WaterlevelAuto->setOffBgBrush(QColor(36,61,91));
        //自动水位是否开启按钮
        connect(ui->btn_WaterlevelAuto,&SlideButtonLib::isOffValueState,this,[=](bool isOff){
            submitControlWrite(QStringLiteral("WaterLevelAuto"), isOff ? 0 : 1, [this, isOff]() {
                ui->waterlevelAutoValue->setText(isOff ? QStringLiteral("OFF") : QStringLiteral("ON"));
                systemState->setStateWaterLevel(isOff ? QStringLiteral("未开启") : QStringLiteral("已开启"));
            });
        });
        connect(ui->SuLvBox_btn,&QPushButton::clicked,this,[=]{
            const int value = ui->SuLvBox->currentIndex();
            const QString text = ui->SuLvBox->currentText();
            submitControlWrite(QStringLiteral("WaterFlow"), value,
                               [this, text]() { ui->waterflowValue->setText(text); });
        });
    }
    //============== 环境控制 ==============
    {
        ui->btn_HJAuto->setOnBgBrush(QColor(36,61,91));
        ui->btn_HJAuto->setOffBgBrush(QColor(36,61,91));
        //自动环境是否开启按钮
        connect(ui->btn_HJAuto,&SlideButtonLib::isOffValueState,this,[=](bool isOff){
            submitControlWrite(QStringLiteral("EnvironmentAuto"), isOff ? 0 : 1, [this, isOff]() {
                ui->hjAutoValue->setText(isOff ? QStringLiteral("OFF") : QStringLiteral("ON"));
            });
        });
        //电源供电设置
        connect(ui->dyBox_btn,&QPushButton::clicked,this,[=]{
            const int value = ui->dyBox->currentIndex();
            const QString text = ui->dyBox->currentText();
            submitControlWrite(QStringLiteral("PowerMode"), value, [this, text]() {
                ui->dyValue->setText(text);
                systemState->setStateDianYuan(text);
            });
        });
        //通信模式设置
        connect(ui->txBox_btn,&QPushButton::clicked,this,[=]{
            const int value = ui->txBox->currentIndex();
            const QString text = ui->txBox->currentText();
            submitControlWrite(QStringLiteral("CommunicationMode"), value,
                               [this, text]() { ui->txValue->setText(text); });
        });
    }
    //============== 氧气控制 ==============
    {
        ui->btn_O2Auto->setOnBgBrush(QColor(36,61,91));
        ui->btn_O2Auto->setOffBgBrush(QColor(36,61,91));
        //自动氧气是否开启按钮
        connect(ui->btn_O2Auto,&SlideButtonLib::isOffValueState,this,[=](bool isOff){
            submitControlWrite(QStringLiteral("OxygenAuto"), isOff ? 0 : 1, [this, isOff]() {
                ui->O2AutoValue->setText(isOff ? QStringLiteral("OFF") : QStringLiteral("ON"));
                systemState->setStateYangQi(isOff ? QStringLiteral("未开启") : QStringLiteral("已开启"));
            });
        });
        //氧气定时设置
        connect(ui->O2TimeBox_btn,&QPushButton::clicked,this,[=]{
            const int value = ui->O2TimeBox->currentIndex();
            const QString text = ui->O2TimeBox->currentText();
            submitControlWrite(QStringLiteral("OxygenDuration"), value,
                               [this, text]() { ui->O2TimeValue->setText(text); });
        });
        //氧气定次设置
        connect(ui->O2NumBox_btn,&QPushButton::clicked,this,[=]{
            const int value = ui->O2NumBox->currentIndex();
            const QString text = ui->O2NumBox->currentText();
            submitControlWrite(QStringLiteral("OxygenFrequency"), value,
                               [this, text]() { ui->O2NumValue->setText(text); });
        });
    }
    //============== 电池控制 ==============
    {
        ui->btn_TYNAuto->setOnBgBrush(QColor(36,61,91));
        ui->btn_TYNAuto->setOffBgBrush(QColor(36,61,91));
        //太阳能功能是否开启按钮
        connect(ui->btn_TYNAuto,&SlideButtonLib::isOffValueState,this,[=](bool isOff){
            submitControlWrite(QStringLiteral("SolarAuto"), isOff ? 0 : 1, [this, isOff]() {
                ui->tynAutoValue->setText(isOff ? QStringLiteral("OFF") : QStringLiteral("ON"));
                ui->tynAutoLogo->setStyleSheet(isOff
                    ? QStringLiteral("border-image: url(:/ptr/NOtype.png);")
                    : QStringLiteral("border-image: url(:/ptr/OKtype.png);"));
            });
        });
    }

}
//FaultDetect页面初始化
void MainWindow::FaultDetectInit()
{
    myFaultDetect=new FaultDetect;
    // 设置myFaultDetect的大小策略为扩展，以便它可以调整大小
    myFaultDetect->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // 获取BottomWidget4的布局（假设它使用QVBoxLayout）
    QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(ui->BottomWidget4->layout());
    if (layout) {
        // 将myFaultDetect添加到布局中
        layout->addWidget(myFaultDetect);
    } else {
        // 如果BottomWidget4没有布局，则创建一个并设置
        layout = new QVBoxLayout(ui->BottomWidget4);
        layout->addWidget(myFaultDetect);
    }
    // 设置BottomWidget4的布局
    ui->BottomWidget4->setLayout(layout);
}

void MainWindow::ModbusInit()
{
    qRegisterMetaType<MonitorSnapshot>("MonitorSnapshot");
    qRegisterMetaType<ConnectionState>("ConnectionState");
    qRegisterMetaType<DeviceState>("DeviceState");

    m_modbusThread = new QThread(this);
    m_modbusManager = new ModbusManager(QString());
    m_modbusManager->moveToThread(m_modbusThread);

    connect(this, &MainWindow::startModbus,
            m_modbusManager, &ModbusManager::start, Qt::QueuedConnection);
    connect(this, &MainWindow::stopModbus,
            m_modbusManager, &ModbusManager::stop, Qt::QueuedConnection);
    connect(this, &MainWindow::controlWriteRequested,
            m_modbusManager, &ModbusManager::enqueueControlWrite, Qt::QueuedConnection);
    connect(m_modbusThread, &QThread::finished,
            m_modbusManager, &QObject::deleteLater);

    connect(m_modbusManager, &ModbusManager::snapshotReady,
            this, &MainWindow::handleSnapshot);
    connect(m_modbusManager, &ModbusManager::connectionStateChanged, this,
            [this](ConnectionState state, const QString &detail) {
                m_modbusOnline = state == ConnectionState::Connected;
                updateConnectionIndicators();
                if (log)
                    log->appendLog(QStringLiteral("Modbus: %1").arg(detail));
            });
    connect(m_modbusManager, &ModbusManager::slaveStateChanged, this,
            [this](int slaveId, DeviceState state, const QString &detail) {
                m_modbusOnline = state == DeviceState::Online;
                updateConnectionIndicators();
                if (log) {
                    log->appendLog(QStringLiteral("Modbus从站%1: %2")
                                       .arg(slaveId).arg(detail));
                }
            });
    connect(m_modbusManager, &ModbusManager::writeRequestFinished, this,
            [this](quint64 requestId, bool success, const QString &detail) {
                const auto pending = m_pendingControlUpdates.find(requestId);
                if (pending == m_pendingControlUpdates.end())
                    return;
                if (success)
                    pending.value()();
                else if (log)
                    log->appendLog(QStringLiteral("控制写入失败: %1").arg(detail));
                m_pendingControlUpdates.erase(pending);
            });
    connect(m_modbusManager, &ModbusManager::configurationError, this,
            [this](const QString &detail) {
                if (log)
                    log->appendLog(QStringLiteral("Modbus配置错误: %1").arg(detail));
            });

    m_modbusThread->start();
    emit startModbus();
}

void MainWindow::DatabaseRuntimeInit()
{
    m_databaseOnline = QSqlDatabase::database().isOpen();

    QString configPath = QDir::current().filePath(QStringLiteral("modbus.ini"));
    if (!QFileInfo::exists(configPath)) {
        configPath = QDir(QCoreApplication::applicationDirPath())
                         .filePath(QStringLiteral("../modbus.ini"));
    }
    QSettings settings(configPath, QSettings::IniFormat);
    const int storageInterval = settings.value(QStringLiteral("Polling/storageIntervalMs"), 5000).toInt();

    m_storageTimer = new QTimer(this);
    m_storageTimer->setInterval(qMax(1000, storageInterval));
    connect(m_storageTimer, &QTimer::timeout, this, &MainWindow::storeLatestSnapshot);
    m_storageTimer->start();

    m_databaseReconnectTimer = new QTimer(this);
    m_databaseReconnectTimer->setInterval(30000);
    connect(m_databaseReconnectTimer, &QTimer::timeout, this, [this]() {
        QSqlDatabase database = QSqlDatabase::database();
        if (database.isOpen() || database.open()) {
            m_databaseOnline = true;
            m_databaseReconnectTimer->stop();
            updateConnectionIndicators();
            if (log)
                log->appendLog(QStringLiteral("数据库连接已恢复"));
        }
    });
    if (!m_databaseOnline)
        m_databaseReconnectTimer->start();
    updateConnectionIndicators();
}

void MainWindow::handleSnapshot(const MonitorSnapshot &snapshot)
{
    m_latestSnapshot = snapshot;
    m_hasSnapshot = true;

    // The worker timestamps the completed reply; this measures queued delivery plus UI entry latency.
    if (snapshot.responseTimestampMs > 0) {
        const qint64 latency = QDateTime::currentMSecsSinceEpoch() - snapshot.responseTimestampMs;
        if (latency > 200)
            qWarning() << "Modbus UI refresh latency exceeded 200 ms:" << latency;
    }

    waterTop->setValue(snapshot);
    waterBottom->setValue(snapshot);
    graphData->updateSnapshot(snapshot);

    const auto display = [&snapshot](MonitorPoint point, const QString &unit) {
        if (snapshot.quality(point) == DataQuality::Bad)
            return QStringLiteral("--");
        QString value = QString::number(snapshot.value(point), 'f', 2);
        if (snapshot.quality(point) == DataQuality::Stale)
            value.append(QStringLiteral(" (旧)"));
        return value + unit;
    };
    ui->tempValue->setText(display(MonitorPoint::Temp, QStringLiteral("℃")));
    ui->tempValue_2->setText(display(MonitorPoint::Temp1, QStringLiteral("℃")));
    ui->tempValue_4->setText(display(MonitorPoint::Temp2, QStringLiteral("℃")));
    ui->waterlevel1Value->setText(display(MonitorPoint::WaterLevel1, QStringLiteral("cm")));
    ui->waterlevel2Value->setText(display(MonitorPoint::WaterLevel2, QStringLiteral("cm")));

    bool anyOnline = false;
    for (const DeviceStatistics &device : snapshot.devices)
        anyOnline = anyOnline || device.state == DeviceState::Online;
    m_modbusOnline = anyOnline;
    updateConnectionIndicators();
}

quint64 MainWindow::submitControlWrite(const QString &controlName,
                                       quint16 value,
                                       const std::function<void()> &onSuccess)
{
    const quint64 requestId = m_nextControlRequestId++;
    m_pendingControlUpdates.insert(requestId, onSuccess);
    emit controlWriteRequested(controlName, value, requestId);
    return requestId;
}

void MainWindow::storeLatestSnapshot()
{
    if (!m_hasSnapshot)
        return;

    QSqlDatabase database = QSqlDatabase::database();
    if (!database.isOpen()) {
        m_databaseOnline = false;
        if (!m_databaseReconnectTimer->isActive())
            m_databaseReconnectTimer->start();
        updateConnectionIndicators();
        return;
    }

    QSqlQuery query(database);
    query.prepare(QStringLiteral(
        "INSERT INTO MonitorData "
        "(Temp,Humidity,Pressure,VOC,Light,PH,TDS,EC,Turbidity,Temp1,Temp2,WaterLevel1,WaterLevel2) "
        "VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?)"));
    for (int index = 0; index < static_cast<int>(MonitorPoint::Count); ++index) {
        const MonitorPoint point = static_cast<MonitorPoint>(index);
        if (m_latestSnapshot.quality(point) == DataQuality::Good)
            query.addBindValue(m_latestSnapshot.value(point));
        else
            query.addBindValue(QVariant(QVariant::Double));
    }

    if (!query.exec()) {
        qWarning() << "MonitorData insert failed:" << query.lastError().text();
        database.close();
        m_databaseOnline = false;
        if (!m_databaseReconnectTimer->isActive())
            m_databaseReconnectTimer->start();
    } else {
        m_databaseOnline = true;
    }
    updateConnectionIndicators();
}

void MainWindow::updateConnectionIndicators()
{
    if (systemState)
        systemState->updateYJPCType(m_modbusOnline ? 1 : 0, m_databaseOnline ? 1 : 0);
}




