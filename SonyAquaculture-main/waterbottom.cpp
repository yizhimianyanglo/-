#include "waterbottom.h"
#include "ui_waterbottom.h"
#include<QDebug>

namespace {

QString displayValue(double value)
{
    QString text = QString::number(value, 'f', 2);
    while (text.contains('.') && text.endsWith('0'))
        text.chop(1);
    if (text.endsWith('.'))
        text.chop(1);
    return text;
}

QString unavailableStyle(const QWidget *widget)
{
    return QStringLiteral("#%1{background:rgba(90,96,105,0.55);}")
        .arg(widget->objectName());
}

} // namespace
WaterBottom::WaterBottom(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WaterBottom)
{
    ui->setupUi(this);
}

WaterBottom::~WaterBottom()
{
    delete ui;
}
//通过值来判断对应范围数值的背景颜色：正常、提示、警告
void WaterBottom::ValueColorIsChanged(double nowValue, double zcValue1, double zcValue2,
                                      double infoValue1, double infoValue2,double infoValue3,double infoValue4, QWidget *widget)
{
    QString objName=widget->objectName();
    //径向渐变（radial gradient） 背景
    QString InfromtionQss=QString("#%1{background: qradialgradient(cx: 0.5, cy: 0.5, radius: 1, fx: 0.5, fy: 0.5, stop: 0 rgba(7, 46, 125, 0), stop: 1 rgba(183, 172, 31, 0.6));}").arg(objName);
    QString WarningQss=QString("#%1{background: qradialgradient(cx: 0.5, cy: 0.5, radius: 1, fx: 0.5, fy: 0.5, stop: 0 rgba(7, 46, 125, 0), stop: 1 #d5442b);}").arg(objName);
    if(nowValue>=zcValue1&&nowValue<=zcValue2)
    {
        widget->setStyleSheet("");
    }
    else if((nowValue>=infoValue1&&nowValue<infoValue2) ||(nowValue>infoValue3&&nowValue<=infoValue4))
    {
        widget->setStyleSheet(InfromtionQss);
        emit writeLog(objName.split("Widget").at(0),PROMPT);
    }
    else {
        widget->setStyleSheet(WarningQss);
        emit writeLog(objName.split("Widget").at(0),WARN);
    }
}
//设置值
void WaterBottom::setValue(const MonitorSnapshot &snapshot)
{
    const auto updatePoint = [this, &snapshot](MonitorPoint point, QLabel *label, QWidget *widget,
                                               double normalLow, double normalHigh,
                                               double promptLow, double promptHigh,
                                               double promptUpperLow, double promptUpperHigh) {
        const DataQuality quality = snapshot.quality(point);
        if (quality == DataQuality::Bad) {
            label->setText(QStringLiteral("--"));
            widget->setStyleSheet(unavailableStyle(widget));
            return;
        }

        const double value = snapshot.value(point);
        label->setText(displayValue(value));
        if (quality == DataQuality::Stale) {
            widget->setStyleSheet(unavailableStyle(widget));
            return;
        }
        ValueColorIsChanged(value, normalLow, normalHigh, promptLow, promptHigh,
                            promptUpperLow, promptUpperHigh, widget);
    };

    updatePoint(MonitorPoint::Ph, ui->PHValue, ui->PHWidget, 6.5, 8.5, 6.0, 6.5, 8.5, 9.0);
    updatePoint(MonitorPoint::Tds, ui->TdsValue, ui->TdsWidget, 200, 500, 150, 200, 500, 800);
    updatePoint(MonitorPoint::Ec, ui->EcValue, ui->EcWidget, 1.5, 3.0, 1.0, 1.5, 3.0, 4.0);
    updatePoint(MonitorPoint::Turbidity, ui->TurbidityValue, ui->TurbidityWidget, 0, 10, 10, 20, 10, 20);
    updatePoint(MonitorPoint::Temp1, ui->Temp1Value, ui->Temp1Widget, 20, 30, 15, 20, 30, 35);
    updatePoint(MonitorPoint::Temp2, ui->Temp2Value, ui->Temp2Widget, 20, 30, 15, 20, 30, 35);
    updatePoint(MonitorPoint::WaterLevel1, ui->WaterLevel1Value, ui->WaterLevel1Widget, 25, 35, 15, 20, 40, 45);
    updatePoint(MonitorPoint::WaterLevel2, ui->WaterLevel2Value, ui->WaterLevel2Widget, 25, 35, 15, 20, 40, 45);
}
