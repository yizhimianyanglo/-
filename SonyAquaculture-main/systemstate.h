#ifndef SYSTEMSTATE_H
#define SYSTEMSTATE_H

#include <QWidget>

namespace Ui {
class SystemState;
}

class SystemState : public QWidget
{
    Q_OBJECT

public:
    explicit SystemState(QWidget *parent = nullptr);
    ~SystemState();
    void setStateFeeding(QString type); // 设置喂食状态
    void setStateTemp(QString type); // 设置温度
    void setStateWaterLevel(QString type); //水位
    void setStateYangQi(QString type); // 氧气
    void setStateDianYuan(QString type);//电源
public slots:
    void updateYJPCType(int YJfd,int PCfd);
private:
    Ui::SystemState *ui;
};

#endif // SYSTEMSTATE_H
