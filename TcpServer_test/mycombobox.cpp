#include "mycombobox.h"

#include <QMouseEvent>

//QComboBox(parent) 调用基类 QComboBox 的构造函数，并将 parent 参数传递给它。 初始化列表 (QComboBox(parent))
MycomboBox::MycomboBox(QWidget *parent):QComboBox(parent)
{

}

void MycomboBox::mousePressEvent(QMouseEvent *event)
{
    //当鼠标左键被按下
    if(event->button() == Qt::LeftButton)
    {
        emit on_ComboBox_clicked();
    }
    QComboBox::mousePressEvent(event); // 保留原有行为
}
