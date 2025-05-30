#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include<QTcpServer>
#include"mycombobox.h"

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

    QTcpServer *server;

public slots:
    void on_newClient_connect();
    void on_readyRead_handler();
    void mdisconnected();
    void mComboBox_refresh(); //鼠标按下事件槽函数

private slots:
    void on_btnListen_clicked();

    void on_btnSend_clicked();

    void on_comboBoxChildren_activated(int index);

    void on_btnStopListen_clicked();

    void on_btnLineout_clicked();

    void on_pushButton_clear_clicked();

private:
    Ui::Widget *ui;
    int childIndex;
};
#endif // WIDGET_H
