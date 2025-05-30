#include "widget.h"
#include "ui_widget.h"

#include <QMessageBox>
#include <QTimer>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    client=new QTcpSocket(this);

    //客户端检测 当有新的可读数据到达 socket 时（即服务器发来了数据），这个信号就会被触发。
    connect(client,&QAbstractSocket::readyRead,this,&Widget::m_Read_data_From_Server);


    //判断客户端是否已经成功连接到服务器
    connect(client,&QTcpSocket::connected,this,&Widget::on_Connected);
}

Widget::~Widget()
{
    delete ui;
}

//连接按钮
void Widget::on_btnConnect_clicked()
{
    //connectToHost(...) 是 QTcpSocket 提供的方法，用来尝试连接远程主机。让客户端连接到指定 IP 和端口的服务器
    client->connectToHost(QHostAddress(ui->lineEdit_IPAddr->text()),ui->lineEdit_Port->text().toInt());

   /* 调用 connectToHost(...) 后，不会立即完成连接。 它只是开始尝试连接，真正的连接建立是异步进行的
紧接着调用 client->state()，此时可能返回： QAbstractSocket::ConnectingState（正在连接） 或者还没开始连接，状态仍是 UnconnectedState*/

   time=new QTimer(this);
    time->setSingleShot(true); //设置单次定时器

    //判断客户端是否已经成功连接到服务器
    /*这行代码放在槽函数时每点击一次连接就会再次连接槽函数 形成一个信号绑定多个槽函数 导致重复打印
    信号触发时，所有已连接的槽函数都会被调用。
    如果用户多次点击按钮，on_Connected 会被多次绑定到 connected 信号，最终导致 重复执行（例如多次打印“连接成功”）。*/
    //connect(client,&QTcpSocket::connected,this,&Widget::on_Connected);



    connect(client,&QAbstractSocket::errorOccurred,this,&Widget::onErroe);
    this->setEnabled(false);

    connect(time,&QTimer::timeout,this,&Widget::onTimeOut);
    time->start(5000);


}

//当有新的可读数据到达 socket 时（即服务器发来了数据），这个槽函数就会被触发。
void Widget::m_Read_data_From_Server()
{
    QTextCursor cursor=ui->textEdit_Rev->textCursor(); //获取文本光标

    QTextCharFormat format;//创建一个字符格式对象； 可用于设置字体、颜色、背景色、粗体、斜体等。
    format.setForeground(QBrush(QColor(Qt::black))); //设置前景色
    cursor.setCharFormat(format);
    /*
    对 QTextCursor 设置了字符格式（setCharFormat()），接下来用这个 cursor 插入的文本就会应用这个格式
        QTextCursor 是一个“带状态”的操作对象；
            你调用 cursor.setCharFormat(format) 之后，插入的新文本会自动带上这个格式；*/

    //ursor 是一个独立拷贝，它不会自动同步 UI 上的光标变化。插入后要更新光标位置
    cursor.insertText(client->readAll());//光标位置决定了插入的位置。

  //  cursor.setCharFormat(QTextCharFormat());// 清除格式

    ui->textEdit_Rev->moveCursor(QTextCursor::End);
    ui->textEdit_Rev->ensureCursorVisible();

        //接收服务器发送来的数据
       // ui->textEdit_Rev->insertPlainText(QString::fromUtf8(client->readAll()));
}


//向服务端发送数据
void Widget::on_pushButton_clicked()
{
     /* 在 TCP 协议中，通信是双向的：
       客户端通过 connectToHost(...) 主动连接服务器；
         连接建立后，客户端和服务器都可以通过各自的 QTcpSocket 对象进行 读写操作；
        当一方调用 write(...) 发送数据时，另一方就会在对应的 socket 上触发 readyRead 信号*/
    client->write(ui->textEdit_Send->toPlainText().toUtf8());

     QTextCursor cursor= ui->textEdit_Rev->textCursor(); //文本光标位置

    QTextCharFormat format; //创建一个字符格式对象； 可用于设置字体、颜色、背景色、粗体、斜体等。
    format.setForeground(QBrush(QColor(Qt::red)));//设置前景色（即字体颜色）为红色；QBrush(QColor(Qt::red)) 是 Qt 的画刷类，用于填充颜色。
    cursor.setCharFormat(format);// 将设置好的格式应用到光标位置；

    cursor.insertText(ui->textEdit_Send->toPlainText().toUtf8());
    ui->textEdit_Rev->moveCursor(QTextCursor::End);
    ui->textEdit_Rev->ensureCursorVisible();
}

//断开连接
void Widget::on_btndiscon_clicked()
{
    client->close();
    //client->deleteLater();
    ui->textEdit_Rev->append("中止连接！");
    ui->btnConnect->setEnabled(true);
    ui->lineEdit_IPAddr->setEnabled(true);
    ui->lineEdit_Port->setEnabled(true);
    ui->btndiscon->setEnabled(false);
}

//连接成功槽函数
void Widget::on_Connected()
{
    time->stop();
    ui->textEdit_Rev->append("连接成功！");
    ui->btnConnect->setEnabled(false);
    ui->lineEdit_IPAddr->setEnabled(false);
    ui->lineEdit_Port->setEnabled(false);
    ui->btndiscon->setEnabled(true);
    //获取对面的IP地址和端口号
    //ui->textEdit_Rev->insertPlainText(client->peerAddress().toString()+" "+QString::number(client->peerPort()));

    this->setEnabled(true);
}

//判断来连接错误的情况
void Widget::onErroe(QAbstractSocket::SocketError error)
{
    qDebug()<<"连接错误"<<error;
    on_btndiscon_clicked();
    this->setEnabled(true);
}

//连接超时
void Widget::onTimeOut()
{
    ui->textEdit_Rev->insertPlainText("连接超时！");
    client->abort();//放弃当前连接 立即终止当前连接并重置 socket 状态
    this->setEnabled(true);
}


void Widget::on_pushButton_clear_clicked()
{
    int ret=QMessageBox::warning(this,tr("询问"),tr("是否清空接收？"),QMessageBox::Yes | QMessageBox::No);
    if(ret==QMessageBox::Yes)
    {
        ui->textEdit_Rev->clear();
    }
    else return;
}

