#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include<QTcpSocket>

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_btnConnect_clicked();
    void m_Read_data_From_Server();

    void on_pushButton_clicked();

    void on_btndiscon_clicked();

    void on_Connected();

    void onErroe(QAbstractSocket::SocketError);

    void onTimeOut();

    void on_pushButton_clear_clicked();

private:
    Ui::Widget *ui;
    QTcpSocket *client;
    QTimer *time;
};
#endif // WIDGET_H
