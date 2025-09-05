/********************************************************************************
** Form generated from reading UI file 'FFMediaPlayer_64.ui'
**
** Created by: Qt User Interface Compiler version 6.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FFMEDIAPLAYER_64_H
#define UI_FFMEDIAPLAYER_64_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_FFMediaPlayer_64Class
{
public:
    QGridLayout *gridLayout_2;
    QWidget *Show;
    QListView *listView;
    QWidget *widget;
    QGridLayout *gridLayout;
    QSlider *horizontalSlider;
    QPushButton *Play;
    QLabel *TotalTime;
    QLabel *Time;
    QLabel *Curr;
    QPushButton *high;
    QPushButton *low;
    QPushButton *SavePic;
    QPushButton *Speed1;
    QPushButton *Speed1_25;
    QLabel *Total_2;
    QSlider *horizontalSlider_2;
    QSpacerItem *horizontalSpacer;
    QPushButton *AddMedia;

    void setupUi(QWidget *FFMediaPlayer_64Class)
    {
        if (FFMediaPlayer_64Class->objectName().isEmpty())
            FFMediaPlayer_64Class->setObjectName("FFMediaPlayer_64Class");
        FFMediaPlayer_64Class->resize(1362, 839);
        gridLayout_2 = new QGridLayout(FFMediaPlayer_64Class);
        gridLayout_2->setSpacing(6);
        gridLayout_2->setContentsMargins(11, 11, 11, 11);
        gridLayout_2->setObjectName("gridLayout_2");
        Show = new QWidget(FFMediaPlayer_64Class);
        Show->setObjectName("Show");
        Show->setMinimumSize(QSize(1280, 720));
        Show->setMaximumSize(QSize(16777215, 16777215));

        gridLayout_2->addWidget(Show, 0, 0, 1, 1);

        listView = new QListView(FFMediaPlayer_64Class);
        listView->setObjectName("listView");
        listView->setMaximumSize(QSize(200, 16777215));

        gridLayout_2->addWidget(listView, 0, 1, 1, 1);

        widget = new QWidget(FFMediaPlayer_64Class);
        widget->setObjectName("widget");
        widget->setMaximumSize(QSize(16777215, 95));
        gridLayout = new QGridLayout(widget);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName("gridLayout");
        horizontalSlider = new QSlider(widget);
        horizontalSlider->setObjectName("horizontalSlider");
        horizontalSlider->setOrientation(Qt::Orientation::Horizontal);

        gridLayout->addWidget(horizontalSlider, 0, 0, 1, 13);

        Play = new QPushButton(widget);
        Play->setObjectName("Play");
        Play->setMaximumSize(QSize(50, 50));

        gridLayout->addWidget(Play, 1, 0, 1, 1);

        TotalTime = new QLabel(widget);
        TotalTime->setObjectName("TotalTime");

        gridLayout->addWidget(TotalTime, 1, 1, 1, 1);

        Time = new QLabel(widget);
        Time->setObjectName("Time");

        gridLayout->addWidget(Time, 1, 2, 1, 1);

        Curr = new QLabel(widget);
        Curr->setObjectName("Curr");

        gridLayout->addWidget(Curr, 1, 3, 1, 1);

        high = new QPushButton(widget);
        high->setObjectName("high");
        high->setMaximumSize(QSize(100, 100));

        gridLayout->addWidget(high, 1, 4, 1, 1);

        low = new QPushButton(widget);
        low->setObjectName("low");
        low->setMaximumSize(QSize(100, 100));

        gridLayout->addWidget(low, 1, 5, 1, 1);

        SavePic = new QPushButton(widget);
        SavePic->setObjectName("SavePic");
        SavePic->setMaximumSize(QSize(50, 50));

        gridLayout->addWidget(SavePic, 1, 6, 1, 1);

        Speed1 = new QPushButton(widget);
        Speed1->setObjectName("Speed1");
        Speed1->setMaximumSize(QSize(50, 50));

        gridLayout->addWidget(Speed1, 1, 7, 1, 1);

        Speed1_25 = new QPushButton(widget);
        Speed1_25->setObjectName("Speed1_25");
        Speed1_25->setMaximumSize(QSize(50, 50));

        gridLayout->addWidget(Speed1_25, 1, 8, 1, 1);

        Total_2 = new QLabel(widget);
        Total_2->setObjectName("Total_2");
        Total_2->setMaximumSize(QSize(50, 50));
        Total_2->setPixmap(QPixmap(QString::fromUtf8("../../../\350\247\206\351\242\221\346\222\255\346\224\276\345\231\250/Media-Build/voice (2).png")));

        gridLayout->addWidget(Total_2, 1, 9, 1, 1);

        horizontalSlider_2 = new QSlider(widget);
        horizontalSlider_2->setObjectName("horizontalSlider_2");
        horizontalSlider_2->setMaximumSize(QSize(269, 16777215));
        horizontalSlider_2->setOrientation(Qt::Orientation::Horizontal);

        gridLayout->addWidget(horizontalSlider_2, 1, 10, 1, 1);

        horizontalSpacer = new QSpacerItem(392, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        gridLayout->addItem(horizontalSpacer, 1, 11, 1, 1);

        AddMedia = new QPushButton(widget);
        AddMedia->setObjectName("AddMedia");

        gridLayout->addWidget(AddMedia, 1, 12, 1, 1);


        gridLayout_2->addWidget(widget, 1, 0, 1, 1);


        retranslateUi(FFMediaPlayer_64Class);

        QMetaObject::connectSlotsByName(FFMediaPlayer_64Class);
    } // setupUi

    void retranslateUi(QWidget *FFMediaPlayer_64Class)
    {
        FFMediaPlayer_64Class->setWindowTitle(QCoreApplication::translate("FFMediaPlayer_64Class", "FFMediaPlayer_64", nullptr));
        Play->setText(QString());
        TotalTime->setText(QCoreApplication::translate("FFMediaPlayer_64Class", "00:00", nullptr));
        Time->setText(QCoreApplication::translate("FFMediaPlayer_64Class", "/", nullptr));
        Curr->setText(QCoreApplication::translate("FFMediaPlayer_64Class", "00:00", nullptr));
        high->setText(QCoreApplication::translate("FFMediaPlayer_64Class", "720P", nullptr));
        low->setText(QCoreApplication::translate("FFMediaPlayer_64Class", "480P", nullptr));
        SavePic->setText(QCoreApplication::translate("FFMediaPlayer_64Class", "\346\210\252\345\233\276", nullptr));
        Speed1->setText(QCoreApplication::translate("FFMediaPlayer_64Class", "1.0", nullptr));
        Speed1_25->setText(QCoreApplication::translate("FFMediaPlayer_64Class", "1.25", nullptr));
        Total_2->setText(QString());
        AddMedia->setText(QCoreApplication::translate("FFMediaPlayer_64Class", "\346\267\273\345\212\240\350\247\206\351\242\221", nullptr));
    } // retranslateUi

};

namespace Ui {
    class FFMediaPlayer_64Class: public Ui_FFMediaPlayer_64Class {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FFMEDIAPLAYER_64_H
