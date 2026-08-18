#include "realtimegraphdata.h"

#include <QChart>
#include <QChartView>
#include <QLegend>
#include <QSplineSeries>
#include <QValueAxis>

using namespace QtCharts;

GraphData::GraphData(QObject *parent)
    : QObject(parent)
    , m_history(static_cast<int>(MonitorPoint::Count))
{
}

void GraphData::setupChart(QChart *chart, QChartView *chartView, const QString &title)
{
    m_chart = chart;
    m_chartView = chartView;
    m_axisX = new QValueAxis(m_chart);
    m_axisX->setRange(1, 10);
    m_axisX->setTickCount(10);
    m_axisY = new QValueAxis(m_chart);
    m_axisY->setTickCount(10);
    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_chart->addAxis(m_axisY, Qt::AlignLeft);

    m_chart->setAnimationOptions(QChart::NoAnimation);// SeriesAnimations
    m_chart->setBackgroundBrush(QColor(27, 49, 73));
    m_chart->legend()->setVisible(true);
    m_chart->legend()->setAlignment(Qt::AlignBottom);
    m_chart->legend()->setLabelColor(QColor(122, 175, 227));
    m_chart->legend()->setFont(QFont(QStringLiteral("楷体"), 14));
    m_chart->setTitleFont(QFont(QStringLiteral("楷体"), 16, QFont::Bold));
    m_chart->setTitleBrush(QColor(122, 175, 227));

    for (QAbstractAxis *axis : m_chart->axes()) {
        axis->setLabelsFont(QFont(QStringLiteral("楷体"), 12));
        axis->setLabelsColor(QColor(122, 175, 227));
        axis->setLinePen(QPen(QColor(14, 26, 50), 2));
        axis->setGridLineColor(QColor(14, 26, 50));
    }
    m_axisX->setLabelsAngle(5);
    m_axisY->setLabelsAngle(-5);
    m_chartView->setChart(m_chart);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    setView(title);
}

void GraphData::setView(const QString &title)
{
    if (!m_chart)
        return;
    m_chart->setTitle(title);
    configureView(title);
    rebuildSeries();
}

void GraphData::updateSnapshot(const MonitorSnapshot &snapshot)
{
    // Do not append stale values; this keeps the trend based on verified samples only.
    for (int index = 0; index < static_cast<int>(MonitorPoint::Count); ++index) {
        const MonitorPoint point = static_cast<MonitorPoint>(index);
        if (snapshot.quality(point) != DataQuality::Good)
            continue;
        QVector<double> &history = m_history[index];
        history.append(snapshot.value(point));
        if (history.size() > 10)
            history.remove(0, history.size() - 10);
    }
    refreshSeries();
}

void GraphData::configureView(const QString &title)
{
    m_activeSpecs.clear();
    if (title == QStringLiteral("温度趋势图")) {
        m_activeSpecs.append(SeriesSpec(MonitorPoint::Temp, QStringLiteral("水上温度"), QColor(137, 104, 205)));
        m_activeSpecs.append(SeriesSpec(MonitorPoint::Temp1, QStringLiteral("水下温度1"), QColor(144, 238, 144)));
        m_activeSpecs.append(SeriesSpec(MonitorPoint::Temp2, QStringLiteral("水下温度2"), QColor(255, 140, 0)));
        m_axisY->setRange(10, 35);
    } else if (title == QStringLiteral("湿度趋势图")) {
        m_activeSpecs.append(SeriesSpec(MonitorPoint::Humidity, QStringLiteral("水上湿度"), QColor(152, 245, 255)));
        m_axisY->setRange(50, 90);
    } else if (title == QStringLiteral("气压趋势图")) {
        m_activeSpecs.append(SeriesSpec(MonitorPoint::Pressure, QStringLiteral("水上气压"), Qt::white));
        m_axisY->setRange(1000, 1030);
    } else if (title == QStringLiteral("VOC趋势图")) {
        m_activeSpecs.append(SeriesSpec(MonitorPoint::Voc, QStringLiteral("水上VOC"), QColor(25, 140, 0)));
        m_axisY->setRange(0, 40);
    } else if (title == QStringLiteral("光照趋势图")) {
        m_activeSpecs.append(SeriesSpec(MonitorPoint::Light, QStringLiteral("水上光照强度"), QColor(255, 250, 205)));
        m_axisY->setRange(300, 1500);
    } else if (title == QStringLiteral("PH趋势图")) {
        m_activeSpecs.append(SeriesSpec(MonitorPoint::Ph, QStringLiteral("水下PH"), QColor(104, 34, 139)));
        m_axisY->setRange(6, 9);
    } else if (title == QStringLiteral("TDS趋势图")) {
        m_activeSpecs.append(SeriesSpec(MonitorPoint::Tds, QStringLiteral("水下TDS"), QColor(100, 149, 237)));
        m_axisY->setRange(150, 800);
    } else if (title == QStringLiteral("EC趋势图")) {
        m_activeSpecs.append(SeriesSpec(MonitorPoint::Ec, QStringLiteral("水下EC"), QColor(72, 209, 204)));
        m_axisY->setRange(1, 4);
    } else if (title == QStringLiteral("浊度趋势图")) {
        m_activeSpecs.append(SeriesSpec(MonitorPoint::Turbidity, QStringLiteral("水下浊度"), QColor(139, 105, 105)));
        m_axisY->setRange(0, 20);
    } else {
        m_activeSpecs.append(SeriesSpec(MonitorPoint::WaterLevel1, QStringLiteral("水下水位1"), QColor(240, 255, 255)));
        m_activeSpecs.append(SeriesSpec(MonitorPoint::WaterLevel2, QStringLiteral("水下水位2"), QColor(72, 118, 255)));
        m_axisY->setRange(15, 45);
    }
}

void GraphData::rebuildSeries()
{
    m_chart->removeAllSeries();
    m_series.clear();
    for (const SeriesSpec &spec : m_activeSpecs) {
        QSplineSeries *series = new QSplineSeries(m_chart);
        series->setName(spec.name);
        series->setPen(QPen(spec.color, 4));
        m_chart->addSeries(series);
        series->attachAxis(m_axisX);
        series->attachAxis(m_axisY);
        m_series.append(series);
    }
    refreshSeries();
}

void GraphData::refreshSeries()
{
    for (int seriesIndex = 0; seriesIndex < m_series.size(); ++seriesIndex) {
        const int pointIndex = static_cast<int>(m_activeSpecs.at(seriesIndex).point);
        const QVector<double> &history = m_history.at(pointIndex);
        QList<QPointF> points;
        for (int index = 0; index < history.size(); ++index)
            points.append(QPointF(index + 1, history.at(index)));
        m_series.at(seriesIndex)->replace(points);
    }
}
