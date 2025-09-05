#pragma once

#include <QtWidgets/QWidget>
#include <QStringList>
#include <QStringListModel>
#include <QRandomGenerator>
#include <QTimer>
#include <QString>
#include <QMap>
#include "ui_FFMediaPlayer_64.h"
#include "Media.h"


class FFMediaPlayer_64 : public QWidget
{
    Q_OBJECT

public:
    FFMediaPlayer_64(QWidget *parent = nullptr);
    ~FFMediaPlayer_64();

    void resizeEvent(QResizeEvent* ev) override;

private slots:
    void on_AddMedia_clicked();

    void on_listView_clicked(const QModelIndex& index);

    void on_Play_clicked();

    void on_horizontalSlider_2_valueChanged(int value);

    void on_horizontalSlider_sliderPressed();

    void on_horizontalSlider_sliderReleased();

    void on_horizontalSlider_valueChanged(int value);

    void on_horizontalSlider_actionTriggered(int action);

    void on_Speed1_clicked();

    void on_Speed1_25_clicked();

    void on_high_clicked();

    void on_low_clicked();

    void on_SavePic_clicked();

    void on_horizontalSlider_sliderMoved(int position);

    void Stop();
signals:
    void mySignal();

private:
    QTimer* Timer = NULL;
    Media* media = NULL;
    QMap<int, QString> mediaMap;
    Ui::FFMediaPlayer_64Class ui;
    QStringList list;
    QStringListModel listModel;
    void Update();

    int currIndex = 0;

    State st = NONE;

    int Count = 1;

    int Time = 0;
    int Seconds = 0;// ��
    int Minutes = 0;// ��
    int Play = -1;

    //���/���˵ķ�Χ 0-5
    int FR_Begin = 1;
    int FR_End = 5;

    //����������ͣ��������10-30
    int Stop_Begin = 10;
    int Stop_End = 100;

    // ����������ڲ��Ŷ�òſ�ʼ��
    int Play_Begin = 1;
    int Play_End = 10;

    // ��ͣ��ʱ��Ĭ����0-5����ͣ�������ת
    int Stop_Time_Begin = 1;
    int Stop_Time_End = 5;
};
