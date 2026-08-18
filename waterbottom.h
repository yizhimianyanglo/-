#ifndef WATERBOTTOM_H
#define WATERBOTTOM_H

#include "monitor_types.h"

#include <QWidget>
#define WARN 1
#define PROMPT 2
namespace Ui {
class WaterBottom;
}

class WaterBottom : public QWidget
{
    Q_OBJECT

public:
    explicit WaterBottom(QWidget *parent = nullptr);
    ~WaterBottom();
    //通过值来判断对应范围数值的背景颜色：正常、提示、警告
    void ValueColorIsChanged(double nowValue, double zcValue1, double zcValue2, double infoValue1, double infoValue2,double infoValue3,double infoValue4, QWidget *widget);
public slots:
    //设置值
    // Refresh only good data; stale values keep their last reading without alarming.
    void setValue(const MonitorSnapshot &snapshot);
signals:
    void writeLog(QString name,int type);
private:
    Ui::WaterBottom *ui;
};

#endif // WATERBOTTOM_H
