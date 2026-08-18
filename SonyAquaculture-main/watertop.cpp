#include "watertop.h"
#include "ui_watertop.h"
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
WaterTop::WaterTop(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WaterTop)
{
    ui->setupUi(this);


}

WaterTop::~WaterTop()
{
    delete ui;
}
//通过值来判断对应范围数值的背景颜色：正常、提示、警告
void WaterTop::ValueColorIsChanged(double nowValue, double zcValue1, double zcValue2, double infoValue1, double infoValue2,double infoValue3,double infoValue4, QWidget *widget)
{
    QString objName=widget->objectName();
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
void WaterTop::setValue(const MonitorSnapshot &snapshot)
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

    updatePoint(MonitorPoint::Temp, ui->TempValue, ui->TempWidget, 20, 30, 15, 20, 30, 35);
    updatePoint(MonitorPoint::Humidity, ui->HumidityValue, ui->HumidityWidget, 60, 80, 50, 60, 80, 90);
    updatePoint(MonitorPoint::Pressure, ui->PressureValue, ui->PressureWidget, 1010, 1020, 1000, 1010, 1020, 1030);
    updatePoint(MonitorPoint::Voc, ui->VocValue, ui->VocWidget, 0, 20, 20, 40, 20, 40);
    updatePoint(MonitorPoint::Light, ui->LightValue, ui->LightWidget, 500, 1000, 300, 500, 1000, 1500);
}
