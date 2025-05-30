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
#include <QtWidgets/QComboBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "mycombobox.h"

QT_BEGIN_NAMESPACE

class Ui_Widget
{
public:
    QWidget *layoutWidget;
    QVBoxLayout *verticalLayout;
    QTextEdit *textEditRev;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QLabel *label;
    QComboBox *comboBox_Port;
    QLabel *label_2;
    QComboBox *comboBox_Addr;
    QLabel *label_3;
    QLineEdit *lineEdit_Port;
    QSpacerItem *horizontalSpacer_2;
    QHBoxLayout *horizontalLayout_3;
    QPushButton *btnListen;
    QPushButton *btnStopListen;
    QPushButton *btnLineout;
    QPushButton *pushButton_clear;
    MycomboBox *comboBoxChildren;
    QHBoxLayout *horizontalLayout_2;
    QTextEdit *textEditSend;
    QPushButton *btnSend;

    void setupUi(QWidget *Widget)
    {
        if (Widget->objectName().isEmpty())
            Widget->setObjectName("Widget");
        Widget->resize(674, 476);
        layoutWidget = new QWidget(Widget);
        layoutWidget->setObjectName("layoutWidget");
        layoutWidget->setGeometry(QRect(10, 10, 411, 441));
        verticalLayout = new QVBoxLayout(layoutWidget);
        verticalLayout->setSpacing(1);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setContentsMargins(5, 5, 5, 5);
        textEditRev = new QTextEdit(layoutWidget);
        textEditRev->setObjectName("textEditRev");

        verticalLayout->addWidget(textEditRev);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(1);
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        label = new QLabel(layoutWidget);
        label->setObjectName("label");

        horizontalLayout->addWidget(label);

        comboBox_Port = new QComboBox(layoutWidget);
        comboBox_Port->addItem(QString());
        comboBox_Port->addItem(QString());
        comboBox_Port->setObjectName("comboBox_Port");
        comboBox_Port->setMaximumSize(QSize(100, 16777215));

        horizontalLayout->addWidget(comboBox_Port);

        label_2 = new QLabel(layoutWidget);
        label_2->setObjectName("label_2");

        horizontalLayout->addWidget(label_2);

        comboBox_Addr = new QComboBox(layoutWidget);
        comboBox_Addr->setObjectName("comboBox_Addr");
        comboBox_Addr->setMaximumSize(QSize(200, 16777215));

        horizontalLayout->addWidget(comboBox_Addr);

        label_3 = new QLabel(layoutWidget);
        label_3->setObjectName("label_3");

        horizontalLayout->addWidget(label_3);

        lineEdit_Port = new QLineEdit(layoutWidget);
        lineEdit_Port->setObjectName("lineEdit_Port");
        lineEdit_Port->setMinimumSize(QSize(0, 0));
        lineEdit_Port->setMaximumSize(QSize(100, 16777215));

        horizontalLayout->addWidget(lineEdit_Port);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_2);

        horizontalLayout->setStretch(2, 1);
        horizontalLayout->setStretch(4, 1);
        horizontalLayout->setStretch(6, 1);

        verticalLayout->addLayout(horizontalLayout);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName("horizontalLayout_3");
        btnListen = new QPushButton(layoutWidget);
        btnListen->setObjectName("btnListen");

        horizontalLayout_3->addWidget(btnListen);

        btnStopListen = new QPushButton(layoutWidget);
        btnStopListen->setObjectName("btnStopListen");

        horizontalLayout_3->addWidget(btnStopListen);

        btnLineout = new QPushButton(layoutWidget);
        btnLineout->setObjectName("btnLineout");

        horizontalLayout_3->addWidget(btnLineout);

        pushButton_clear = new QPushButton(layoutWidget);
        pushButton_clear->setObjectName("pushButton_clear");

        horizontalLayout_3->addWidget(pushButton_clear);


        verticalLayout->addLayout(horizontalLayout_3);

        comboBoxChildren = new MycomboBox(layoutWidget);
        comboBoxChildren->setObjectName("comboBoxChildren");

        verticalLayout->addWidget(comboBoxChildren);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        textEditSend = new QTextEdit(layoutWidget);
        textEditSend->setObjectName("textEditSend");
        textEditSend->setMinimumSize(QSize(0, 0));
        textEditSend->setMaximumSize(QSize(16777215, 150));

        horizontalLayout_2->addWidget(textEditSend);

        btnSend = new QPushButton(layoutWidget);
        btnSend->setObjectName("btnSend");
        btnSend->setMinimumSize(QSize(70, 60));

        horizontalLayout_2->addWidget(btnSend);


        verticalLayout->addLayout(horizontalLayout_2);

        verticalLayout->setStretch(0, 5);
        verticalLayout->setStretch(1, 1);
        verticalLayout->setStretch(2, 1);
        verticalLayout->setStretch(4, 3);

        retranslateUi(Widget);

        QMetaObject::connectSlotsByName(Widget);
    } // setupUi

    void retranslateUi(QWidget *Widget)
    {
        Widget->setWindowTitle(QCoreApplication::translate("Widget", "\347\275\221\347\273\234\350\260\203\350\257\225\345\212\251\346\211\213\346\234\215\345\212\241\347\253\257 - \345\260\217\345\221\250\345\207\272\345\223\201", nullptr));
        label->setText(QCoreApplication::translate("Widget", "\351\200\232\344\277\241\345\215\217\350\256\256", nullptr));
        comboBox_Port->setItemText(0, QCoreApplication::translate("Widget", "TCP", nullptr));
        comboBox_Port->setItemText(1, QCoreApplication::translate("Widget", "UDP", nullptr));

        label_2->setText(QCoreApplication::translate("Widget", " \346\234\215\345\212\241\345\231\250IP\345\234\260\345\235\200", nullptr));
        label_3->setText(QCoreApplication::translate("Widget", "\347\253\257\345\217\243\345\217\267", nullptr));
        lineEdit_Port->setText(QCoreApplication::translate("Widget", "8888", nullptr));
        btnListen->setText(QCoreApplication::translate("Widget", "\347\233\221\345\220\254", nullptr));
        btnStopListen->setText(QCoreApplication::translate("Widget", "\345\201\234\346\255\242\347\233\221\345\220\254", nullptr));
        btnLineout->setText(QCoreApplication::translate("Widget", "\346\226\255\345\274\200", nullptr));
        pushButton_clear->setText(QCoreApplication::translate("Widget", "\346\270\205\347\251\272\346\216\245\346\224\266", nullptr));
        btnSend->setText(QCoreApplication::translate("Widget", "\345\217\221\351\200\201", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Widget: public Ui_Widget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WIDGET_H
