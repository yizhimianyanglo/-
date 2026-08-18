#include "mainwindow.h"
#include <QApplication>
#include<QSqlDatabase>
#include<QSqlError>

#include<QDebug>

#include "systemstate.h"
#include "log.h"
#include "waterbottom.h"

int main(int argc, char *argv[])
{
    //永远不应用高分屏及缩放
#if (QT_VERSION >= QT_VERSION_CHECK(5,0,0))
    QApplication::setAttribute(Qt::AA_Use96Dpi);
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5,14,0))
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::Floor);
#endif
    QApplication a(argc, argv);

    qDebug()<< QSqlDatabase::drivers();
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");


    db.setHostName(qEnvironmentVariable("DY_DB_HOST", "127.0.0.1"));
   // db.setPort(3306);//端口号
    db.setDatabaseName("test");//数据库名
    db.setUserName(qEnvironmentVariable("DY_DB_USER", "root"));
    db.setPassword(qEnvironmentVariable("DY_DB_PASSWORD"));
    bool ok = db.open();
    if (ok){
        qDebug()<< "数据库连接成功";
    }
    else {
        // Acquisition must remain available when the historian is temporarily offline.
        qWarning() << "数据库连接失败，程序将继续采集并定时重连:" << db.lastError().text();
    }
    MainWindow w;
     w.show();



     // WaterBottom w;
     // w.show();



    return a.exec();
}
