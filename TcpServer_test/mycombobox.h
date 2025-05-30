#ifndef MYCOMBOBOX_H
#define MYCOMBOBOX_H

#include <QComboBox>
#include <QWidget>

class MycomboBox : public QComboBox
{
    Q_OBJECT
public:
    MycomboBox(QWidget *parent);

protected:
    void mousePressEvent(QMouseEvent *event) override; //重写鼠标按下事件

signals:
    void on_ComboBox_clicked();
};

#endif // MYCOMBOBOX_H
