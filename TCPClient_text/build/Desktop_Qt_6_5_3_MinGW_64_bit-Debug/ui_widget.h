/********************************************************************************
** Form generated from reading UI file 'widget.ui'
**
** Created by: Qt User Interface Compiler version 6.5.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WIDGET_H
#define UI_WIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Widget
{
public:
    QVBoxLayout *verticalLayout;
    QTextEdit *textEdit_Rev;
    QHBoxLayout *horizontalLayout_5;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QLineEdit *lineEdit_IPAddr;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_2;
    QLineEdit *lineEdit_Port;
    QHBoxLayout *horizontalLayout_4;
    QPushButton *btnConnect;
    QPushButton *btndiscon;
    QPushButton *pushButton_clear;
    QHBoxLayout *horizontalLayout_2;
    QTextEdit *textEdit_Send;
    QPushButton *pushButton;

    void setupUi(QWidget *Widget)
    {
        if (Widget->objectName().isEmpty())
            Widget->setObjectName("Widget");
        Widget->resize(568, 497);
        verticalLayout = new QVBoxLayout(Widget);
        verticalLayout->setObjectName("verticalLayout");
        textEdit_Rev = new QTextEdit(Widget);
        textEdit_Rev->setObjectName("textEdit_Rev");

        verticalLayout->addWidget(textEdit_Rev);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setSpacing(1);
        horizontalLayout_5->setObjectName("horizontalLayout_5");
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        label = new QLabel(Widget);
        label->setObjectName("label");
        label->setMaximumSize(QSize(60, 16777215));

        horizontalLayout->addWidget(label);

        lineEdit_IPAddr = new QLineEdit(Widget);
        lineEdit_IPAddr->setObjectName("lineEdit_IPAddr");
        lineEdit_IPAddr->setMaximumSize(QSize(16777215, 16777215));

        horizontalLayout->addWidget(lineEdit_IPAddr);


        horizontalLayout_5->addLayout(horizontalLayout);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(1);
        horizontalLayout_3->setObjectName("horizontalLayout_3");
        label_2 = new QLabel(Widget);
        label_2->setObjectName("label_2");
        label_2->setMaximumSize(QSize(60, 16777215));

        horizontalLayout_3->addWidget(label_2);

        lineEdit_Port = new QLineEdit(Widget);
        lineEdit_Port->setObjectName("lineEdit_Port");
        lineEdit_Port->setMaximumSize(QSize(16777215, 16777215));

        horizontalLayout_3->addWidget(lineEdit_Port);

        horizontalLayout_3->setStretch(0, 1);
        horizontalLayout_3->setStretch(1, 3);

        horizontalLayout_5->addLayout(horizontalLayout_3);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setSpacing(1);
        horizontalLayout_4->setObjectName("horizontalLayout_4");
        btnConnect = new QPushButton(Widget);
        btnConnect->setObjectName("btnConnect");
        btnConnect->setMinimumSize(QSize(80, 0));
        btnConnect->setMaximumSize(QSize(16777215, 16777215));

        horizontalLayout_4->addWidget(btnConnect);

        btndiscon = new QPushButton(Widget);
        btndiscon->setObjectName("btndiscon");
        btndiscon->setMinimumSize(QSize(80, 0));
        btndiscon->setMaximumSize(QSize(16777215, 16777215));

        horizontalLayout_4->addWidget(btndiscon);

        pushButton_clear = new QPushButton(Widget);
        pushButton_clear->setObjectName("pushButton_clear");

        horizontalLayout_4->addWidget(pushButton_clear);


        horizontalLayout_5->addLayout(horizontalLayout_4);

        horizontalLayout_5->setStretch(0, 1);
        horizontalLayout_5->setStretch(1, 1);
        horizontalLayout_5->setStretch(2, 1);

        verticalLayout->addLayout(horizontalLayout_5);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(1);
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        textEdit_Send = new QTextEdit(Widget);
        textEdit_Send->setObjectName("textEdit_Send");

        horizontalLayout_2->addWidget(textEdit_Send);

        pushButton = new QPushButton(Widget);
        pushButton->setObjectName("pushButton");
        pushButton->setMinimumSize(QSize(70, 70));
        pushButton->setMaximumSize(QSize(10000, 70));

        horizontalLayout_2->addWidget(pushButton);

        horizontalLayout_2->setStretch(0, 5);
        horizontalLayout_2->setStretch(1, 1);

        verticalLayout->addLayout(horizontalLayout_2);


        retranslateUi(Widget);

        QMetaObject::connectSlotsByName(Widget);
    } // setupUi

    void retranslateUi(QWidget *Widget)
    {
        Widget->setWindowTitle(QCoreApplication::translate("Widget", "\347\275\221\347\273\234\350\260\203\350\257\225\345\212\251\346\211\213\345\256\242\346\210\267\347\253\257 -\345\260\217\345\221\250\345\207\272\345\223\201", nullptr));
        label->setText(QCoreApplication::translate("Widget", "\346\234\215\345\212\241\347\253\257IP\345\234\260\345\235\200", nullptr));
        lineEdit_IPAddr->setText(QCoreApplication::translate("Widget", "192.168.3.171", nullptr));
        label_2->setText(QCoreApplication::translate("Widget", "\346\234\215\345\212\241\347\253\257\347\253\257\345\217\243", nullptr));
        lineEdit_Port->setText(QCoreApplication::translate("Widget", "8888", nullptr));
        btnConnect->setText(QCoreApplication::translate("Widget", "\350\277\236\346\216\245", nullptr));
        btndiscon->setText(QCoreApplication::translate("Widget", "\346\226\255\345\274\200", nullptr));
        pushButton_clear->setText(QCoreApplication::translate("Widget", "\346\270\205\347\251\272\346\216\245\346\224\266", nullptr));
        pushButton->setText(QCoreApplication::translate("Widget", "\345\217\221\351\200\201", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Widget: public Ui_Widget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WIDGET_H
