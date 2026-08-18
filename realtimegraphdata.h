#ifndef REALTIMEGRAPHDATA_H
#define REALTIMEGRAPHDATA_H

#include "monitor_types.h"

#include <QColor>
#include <QObject>
#include <QVector>

namespace QtCharts {
class QChart;
class QChartView;
class QSplineSeries;
class QValueAxis;
}

class GraphData : public QObject
{
    Q_OBJECT

public:
    explicit GraphData(QObject *parent = nullptr);
    void setupChart(QtCharts::QChart *chart, QtCharts::QChartView *chartView,
                    const QString &title);
    void setView(const QString &title);

public slots:
    void updateSnapshot(const MonitorSnapshot &snapshot);

private:
    struct SeriesSpec
    {
        SeriesSpec(MonitorPoint monitorPoint, const QString &seriesName, const QColor &seriesColor)
            : point(monitorPoint), name(seriesName), color(seriesColor) {}
        MonitorPoint point;
        QString name;
        QColor color;
    };

    void configureView(const QString &title);
    void rebuildSeries();
    void refreshSeries();

    QtCharts::QChart *m_chart = nullptr;
    QtCharts::QChartView *m_chartView = nullptr;
    QtCharts::QValueAxis *m_axisX = nullptr;
    QtCharts::QValueAxis *m_axisY = nullptr;
    QList<QtCharts::QSplineSeries *> m_series;
    QList<SeriesSpec> m_activeSpecs;
    QVector<QVector<double>> m_history;
};

#endif // REALTIMEGRAPHDATA_H
