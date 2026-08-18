#include "log.h"
#include "ui_log.h"
#include <QDateTime>
#include<QDebug>
#include<QScrollBar>
Log::Log(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Log)
{
    ui->setupUi(this);

    ui->tableWidget->viewport()->installEventFilter(this);//为 tableWidget 的“视口”（viewport）安装一个事件过滤器 this，使得你可以拦截并处理发往视口的所有事件
    ui->tableWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);//视图如何在垂直方向滚动其内容物
   //  QAbstractItemView::ScrollPerPixel  1 : 视图将一次滚动一个像素。QAbstractItemView::ScrollPerItem   0: 该视图将一次滚动一个项目。
    QString headerStyle = "QHeaderView::section { background: rgb(3, 57, 103); color: rgb(122, 175, 227); border: 1px solid rgb(14, 26, 50); }";
    ui->tableWidget->horizontalHeader()->setStyleSheet(headerStyle); // 设置水平表头的样式表
    ui->tableWidget->horizontalHeader()->setFont(QFont("楷体", 12));
    ui->tableWidget->horizontalHeader()->setFixedHeight(25);
    ui->tableWidget->setRowCount(10);
    // 设置第一列宽度为固定值，其他列自动适应
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    ui->tableWidget->setColumnWidth(0, 80); // 固定第一列宽度为 80
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch); // 其他列自动调整

    ui->tableWidget->verticalHeader()->hide();
    ui->tableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->tableWidget->setFont(QFont("楷体", 10));
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectItems);//选择行为  QAbstractItemView::SelectItems   QAbstractItemView::SelectRows   QAbstractItemView::SelectColumns
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers); //设置编辑触发器 关闭表格的所有编辑功能
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection); // 设置选择模式
    QString tableStyle = "QTableWidget { color: rgb(122, 175, 227); selection-background-color: rgb(3, 57, 103); selection-color: rgb(122, 175, 227); background-color: rgb(14, 26, 50); gridline-color: rgb(19, 39, 67); }";
    QString itemStyle = "QTableWidget::item:hover { background-color: rgb(19, 39, 67); }";
    ui->tableWidget->setStyleSheet(tableStyle + itemStyle);

    //::handle 是滚动条的“滑块”部分 ::sub-line：向上箭头（垂直滚动条顶部）或 向左箭头（水平滚动条左侧）。 ::add-line：向下箭头（底部）或 向右箭头（右侧）。
//    ::sub-page：点击滑块上方或左侧的轨道区域，会向上/向左翻页。     ::add-page：点击滑块下方或右侧的轨道区域，会向下/向右翻页。
    QString scrollBarStyle = "QScrollBar { background: rgb(14, 26, 50); width: 12px; }"
                             "QScrollBar::handle { background: rgb(19, 39, 67); border: 0px solid rgb(255, 255, 255); border-radius: 5px; }"
                             "QScrollBar::handle:hover { background: rgb(122, 175, 229); }"
                             "QScrollBar::sub-line, QScrollBar::add-line { background: rgb(19, 48, 80); }"
                             "QScrollBar::sub-page:vertical, QScrollBar::add-page:vertical { background: rgb(19, 48, 80); }";
    ui->tableWidget->verticalScrollBar()->setStyleSheet(scrollBarStyle);
    ui->tableWidget->horizontalScrollBar()->setStyleSheet(scrollBarStyle);


    connect(ui->tableWidget, &QTableWidget::itemActivated, this, [=](QTableWidgetItem *item) {
        qDebug() << "双击了:" << item->text()<< endl;
    });

}

Log::~Log()
{
    delete ui;
}

void Log::appendLog(QString msg)
{
    //获取总行数
    int RowCount=ui->tableWidget->rowCount();
//    qDebug()<<"总行数："<<RowCount<<endl;
    QString nowTime=QDateTime::currentDateTime().toString("hh:mm:ss");
    if(logRowCount>RowCount)
    {
        for (int i=0;i<RowCount-1;i++)
        {
            ui->tableWidget->item(i,0)->setText(ui->tableWidget->item(i+1,0)->text());
            ui->tableWidget->item(i,1)->setText(ui->tableWidget->item(i+1,1)->text());
        }
        ui->tableWidget->item(RowCount-1,0)->setText(nowTime);
        ui->tableWidget->item(RowCount-1,1)->setText(msg);
    }
    else if (logRowCount<=RowCount) {
        ui->tableWidget->setItem(logRowCount-1, 0, new QTableWidgetItem(nowTime));
        ui->tableWidget->item(logRowCount-1, 0)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);//将 QTableWidget 中某个单元格内的文本设置为“水平居中 + 垂直居中”对齐。
        ui->tableWidget->setItem(logRowCount-1, 1, new QTableWidgetItem(msg));
        ui->tableWidget->item(logRowCount-1, 1)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        logRowCount++;
    }
}
