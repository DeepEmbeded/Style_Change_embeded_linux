/********************************************************************************
** Form generated from reading UI file 'widget.ui'
**
** Created by: Qt User Interface Compiler version 5.15.10
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WIDGET_H
#define UI_WIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Widget
{
public:
    QHBoxLayout *horizontalLayout;
    QLabel *inferLabel;
    QVBoxLayout *verticalLayout;
    QPushButton *btnStart;
    QPushButton *btnllm;
    QPushButton *SummarizeButton;
    QPushButton *cancelTrackingButton;
    QPushButton *exitbt;

    void setupUi(QWidget *Widget)
    {
        if (Widget->objectName().isEmpty())
            Widget->setObjectName(QString::fromUtf8("Widget"));
        Widget->resize(740, 484);
        horizontalLayout = new QHBoxLayout(Widget);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        inferLabel = new QLabel(Widget);
        inferLabel->setObjectName(QString::fromUtf8("inferLabel"));
        inferLabel->setMinimumSize(QSize(601, 401));

        horizontalLayout->addWidget(inferLabel);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(6);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        btnStart = new QPushButton(Widget);
        btnStart->setObjectName(QString::fromUtf8("btnStart"));
        btnStart->setMaximumSize(QSize(80, 25));

        verticalLayout->addWidget(btnStart);

        btnllm = new QPushButton(Widget);
        btnllm->setObjectName(QString::fromUtf8("btnllm"));
        btnllm->setMaximumSize(QSize(80, 31));

        verticalLayout->addWidget(btnllm);

        SummarizeButton = new QPushButton(Widget);
        SummarizeButton->setObjectName(QString::fromUtf8("SummarizeButton"));
        SummarizeButton->setMaximumSize(QSize(80, 31));

        verticalLayout->addWidget(SummarizeButton);

        cancelTrackingButton = new QPushButton(Widget);
        cancelTrackingButton->setObjectName(QString::fromUtf8("cancelTrackingButton"));
        cancelTrackingButton->setMaximumSize(QSize(80, 31));

        verticalLayout->addWidget(cancelTrackingButton);

        exitbt = new QPushButton(Widget);
        exitbt->setObjectName(QString::fromUtf8("exitbt"));
        exitbt->setMaximumSize(QSize(80, 31));

        verticalLayout->addWidget(exitbt);


        horizontalLayout->addLayout(verticalLayout);


        retranslateUi(Widget);

        QMetaObject::connectSlotsByName(Widget);
    } // setupUi

    void retranslateUi(QWidget *Widget)
    {
        Widget->setWindowTitle(QCoreApplication::translate("Widget", "Widget", nullptr));
        inferLabel->setText(QCoreApplication::translate("Widget", "\346\230\276\347\244\272\345\214\272\345\237\237", nullptr));
        btnStart->setText(QCoreApplication::translate("Widget", "\345\274\200\345\247\213", nullptr));
        btnllm->setText(QCoreApplication::translate("Widget", "\345\260\217\347\221\236", nullptr));
        SummarizeButton->setText(QCoreApplication::translate("Widget", "\346\221\230\350\246\201", nullptr));
        cancelTrackingButton->setText(QCoreApplication::translate("Widget", "\345\217\226\346\266\210\350\267\237\350\270\252", nullptr));
        exitbt->setText(QCoreApplication::translate("Widget", "\351\200\200\345\207\272", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Widget: public Ui_Widget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WIDGET_H
