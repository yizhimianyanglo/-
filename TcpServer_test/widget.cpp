#include "widget.h"
#include "ui_widget.h"

#include<QDebug>
#include <QMessageBox>
#include <QNetworkInterface>
#include <QTcpSocket>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    this->setLayout(ui->verticalLayout);

    ui->btnLineout->setEnabled(false);
    ui->btnStopListen->setEnabled(false);
    ui->btnSend->setEnabled(false);

    /*  客户端：使用 QTcpSocket 连接到服务器，发送和接收数据。
        服务端：使用 QTcpServer 监听端口，接受客户端连接，并通过 QTcpSocket 与客户端通信。*/

    server=new QTcpServer(this);

    //鼠标按下事件刷新
    connect(ui->comboBoxChildren,&MycomboBox::on_ComboBox_clicked,this,&Widget::mComboBox_refresh);


    //&QTcpServer::newConnection：这是一个信号，每当有新的客户端尝试连接服务器时，QTcpServer 就会发出这个信号。
    connect(server,&QTcpServer::newConnection,this,&Widget::on_newClient_connect);

    //QHostAddress 类提供 I P 地址
    //QList<QHostAddress> QNetworkInterface::allAddress()
    //Qnetworklnterface类提供了主机的IP地址和网络接口的列表
    QList<QHostAddress> addresss=QNetworkInterface::allAddresses();
    for(const QHostAddress &tmp:addresss)
    {
        //QHostAddress::protocol() 返回地址的协议类型（QAbstractSocket::IPv4Protocol 或 QAbstractSocket::IPv6Protocol）
        if(tmp.protocol() == QAbstractSocket::IPv4Protocol) //判断是否为 IPv4 地址
            ui->comboBox_Addr->addItem(tmp.toString()); //将本机的Ip地址添加到comboBox
    }



}

Widget::~Widget()
{
    delete ui;
}

// 检测到新的客户端尝试连接服务器时的槽函数
void Widget::on_newClient_connect()
{
    qDebug()<<"newClient In!";//输出调试信息，表示检测到有新客户端连接。

    /* hasPendingConnections() 是 QTcpServer 的成员函数，返回布尔值：
        true 表示当前有未处理的客户端连接请求。
        false 表示没有新连接。

      nextPendingConnection() 是 QTcpServer 的成员函数，返回一个 QTcpSocket* 指针：
      该指针指向一个已建立的 TCP 套接字（QTcpSocket 对象），用于与客户端通信

    当客户端尝试连接到服务器时，QTcpServer 会将这些连接请求放入一个队列中（称为“挂起连接”）。
  nextPendingConnection() 会从队列中取出第一个待处理的连接，并返回一个指向 QTcpSocket 对象的指针，该对象表示与客户端的通信通道。 */

    if(server->hasPendingConnections()==true)
    {

        QTcpSocket *connction=server->nextPendingConnection(); //调用 nextPendingConnection() 获取客户端套接字。
        qDebug()<<"客户端IP地址:"<<connction->peerAddress().toString()<<"客户端的端口号:"<<connction->peerPort();
        ui->textEditRev->insertPlainText("客户端IP地址:"+connction->peerAddress().toString()+
                                         "\n客户端端口号:"+QString::number(connction->peerPort())+"\n");

        //QIODevice::readyRead 用于通知数据可读的信号 表示设备上有可用的数据可以读取
        // 检测到客户端有可读信号   readyRead 是异步信号，不会阻塞程序执行，非常适合 GUI 程序。
        connect(connction,&QIODevice::readyRead,this,&Widget::on_readyRead_handler);


        //当套接字断开连接时，会发出此信号。当客户端与服务端断开连接时，自动调用 Widget 类中的 mdisconnected() 槽函数。
        connect(connction,&QAbstractSocket::disconnected,this,&Widget::mdisconnected);

        //将新建立连接的客户端的端口号加入 ui comboBoxChildren
        ui->comboBoxChildren->addItem(QString::number(connction->peerPort()));
        ui->comboBoxChildren->setCurrentText(QString::number(connction->peerPort()));
        childIndex=ui->comboBoxChildren->count()-1;//下标从0开始 新加入的时候添加到下卡框的末尾



        if(!ui->btnSend->isEnabled()) //有了新的连接 可以点击发送
        {
            ui->btnSend->setEnabled(true);
        }
    }


}

//检测到客户端有可读信号时 槽函数服务端接收客户端数据
void Widget::on_readyRead_handler()
{
    //qobject_cast<T*>(...) 安全地将 QObject* 转换为具体类型指针
    QTcpSocket *socket=qobject_cast<QTcpSocket*>(sender()); //来获取到底是哪个客户端套接字发来的数据。

    QByteArray revData=socket->readAll(); // 读取数据

    ui->textEditRev->insertPlainText(QString::fromUtf8(revData));
    ui->textEditRev->moveCursor(QTextCursor::End); //移动光标到末尾
    ui->textEditRev->ensureCursorVisible();//确保光标可视 在光标被移动到文本末尾后 这行代码确保该位置对用户可见
}

void Widget::mdisconnected()
{
    QTcpSocket *socket=qobject_cast<QTcpSocket*>(sender()); //来获取到底是哪个客户端与服务端断开

    QString conText=QString::number(socket->peerPort()); // 获取断开服务器连接的客户端的端口号
    int tmpIndex=ui->comboBoxChildren->findText(conText); //找到对应名称的索引
    ui->comboBoxChildren->removeItem(tmpIndex); //删除对应索引的端口号

    childIndex=0;
    if(ui->comboBoxChildren->count()>0) //确保有下拉菜单
    ui->comboBoxChildren->setCurrentIndex(0); //显示下拉框文本第一个

    //断开后 如果没有了子客户连接 那么把发送按钮禁用
    if(ui->comboBoxChildren->count()==0) //查找所有满足条件的子对象 如果为空
    {
        ui->btnSend->setEnabled(false);
    }

    ui->textEditRev->insertPlainText("客户端断开!");
    socket->deleteLater();//释放内存
}

void Widget::mComboBox_refresh()
{
    ui->comboBoxChildren->clear();
    QList<QTcpSocket*> tcpSocketClients=server->findChildren<QTcpSocket*>();
    if(tcpSocketClients.size()>0)
    {
        for(auto tmp:tcpSocketClients)
        {
            // tmp->peerPort() 获取该客户端连接使用的端口号（int 类型）
            ui->comboBoxChildren->addItem(QString::number(tmp->peerPort()));
        }
    }
    ui->comboBoxChildren->addItem("All");
}

//监听   listen()
void Widget::on_btnListen_clicked()
{
    //QHostAddress addr("192.168.3.171");//创建一个 QHostAddress 对象，表示服务器要监听的 IP 地址。
    //QHostAddress addr(QHostAddress::Any); // 监听所有 IPv4 接口

    //开始监听
    int port=ui->lineEdit_Port->text().toInt(); //获取端口号
    /*listen()用于启动服务器并开始监听指定的 IP 地址和端口，等待客户端的连接请求
        true：服务器成功启动并开始监听。
        false：启动失败（例如端口被占用、IP地址无效等）。*/
    if (!server->listen(QHostAddress(ui->comboBox_Addr->currentText()), port))
    {
        qDebug() << "Listen failed:" << server->errorString();
        QMessageBox::warning(this,tr("报错"),tr("监听启动失败"));
        return;
    }
    ui->btnListen->setEnabled(false);
    ui->btnLineout->setEnabled(true);
    ui->btnStopListen->setEnabled(true);
}


//服务器向客户端发送数据
void Widget::on_btnSend_clicked()
{
    /*findChild 和 findChildren 是 Qt 提供的两个非常有用的函数，用于在 QObject 树中查找子对象。它们的主要区别在于返回值的数量以及如何处理查找条件。
      1. 基本概念
          findChild：用于查找单个子对象。
         findChildren：用于查找符合查询条件的所有子对象，并返回一个列表。*/

    //当客户端尝试连接到服务器时，QTcpServer 会将这些连接请求放入一个队列中（称为“挂起连接”）
    QList<QTcpSocket*> tcpSocketClients= server->findChildren<QTcpSocket*>();//找所有的QTcpSocket
    if(tcpSocketClients.isEmpty())
    {
        QMessageBox::information(this,tr("提示"),tr("已无连接的客户端"),QMessageBox::Ok);
        ui->btnSend->setEnabled(false);
        return;
    }
    //当用户不选择All 所有客户进行发送的时候
    if(ui->comboBoxChildren->currentText()!="All")
    {
        //根据用户的选择 找到指定客户端进行数据通信，通过childIndex来查找
        tcpSocketClients[childIndex]->write(ui->textEditSend->toPlainText().toUtf8());
    }
    else
    {
        //遍历所有子客户端 并一一调用write函数，向所有客户端发送数据
        for( QTcpSocket* tmp:tcpSocketClients)
        {
           // QTextCodec  *codec=QTextCodec::codecForName("GBK");
            tmp->write(ui->textEditSend->toPlainText().toUtf8());
        }
    }
}

// 获取用户选择哪一个的选项  定义了一个全局变量
void Widget::on_comboBoxChildren_activated(int index)
{
    childIndex=index;
}

//停止监听
void Widget::on_btnStopListen_clicked()
{
    //停止监听前 先关闭所有客户端
    QList<QTcpSocket*> tcpSocketClients=server->findChildren<QTcpSocket*>();
    for(QTcpSocket* tmp:tcpSocketClients)
    {
        tmp->close();
    }
    server->close();
    ui->btnListen->setEnabled(true);
    ui->btnLineout->setEnabled(false);
    ui->btnStopListen->setEnabled(false);
}

//断开
void Widget::on_btnLineout_clicked()
{
    on_btnStopListen_clicked(); //停止监听
    server->deleteLater();
    this->close();
}


void Widget::on_pushButton_clear_clicked()
{
    int ret=QMessageBox::warning(this,tr("询问"),tr("是否清空接收？"),QMessageBox::Yes | QMessageBox::No);
    if(ret==QMessageBox::Yes)
    {
        ui->textEditRev->clear();
    }
    else return;
}

