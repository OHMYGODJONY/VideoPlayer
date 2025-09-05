#include "FFMediaPlayer_64.h"
#include <iostream>
#include <fstream>
#include <QFileDialog>
#include <QMessageBox>

int flag1 = 1;
int flag2 = 1;
using namespace std;

FFMediaPlayer_64::FFMediaPlayer_64(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    media = new Media((void*)ui.Show->winId());
    media->setUpdateCallback(std::bind(&FFMediaPlayer_64::Update, this));
    media->setStopCallback(std::bind(&FFMediaPlayer_64::Stop, this));
    ui.horizontalSlider_2->setRange(0, SDL_MIX_MAXVOLUME);
    ui.horizontalSlider_2->setValue(SDL_MIX_MAXVOLUME / 8);
    QIcon icon1;
    icon1.addFile("./play.png");
    ui.Play->resize(40, 40);
    ui.Play->setIcon(icon1);
    ui.Play->setIconSize(QSize(40, 50));

    this->setWindowTitle("FFMediaPlayer");
    QIcon icon2;
    icon2.addFile("./title.png");
    this->setWindowIcon(icon2);

    ui.Speed1->setStyleSheet("QPushButton { background-color: " + QColor(173, 216, 210).name() + "; }");
    ui.low->hide();
    ui.high->hide();
    Timer = new QTimer(this);
    connect(this, &FFMediaPlayer_64::mySignal, this, &FFMediaPlayer_64::Stop);
    connect(Timer, &QTimer::timeout, this, &FFMediaPlayer_64::Stop);
}

FFMediaPlayer_64::~FFMediaPlayer_64()
{
    delete media;
}

void FFMediaPlayer_64::resizeEvent(QResizeEvent* ev)
{
    std::cout << "height: " << ui.Show->height() << "width: " << ui.Show->width() << std::endl;
    // 先销毁旧的SDL资源
    media->destroySDLRresources();

    // 重新创建SDL资源
    media->createSDLRresources((void*)ui.Show->winId());
    media->set_show_rect(ui.Show->width(), ui.Show->height());
}

void FFMediaPlayer_64::on_AddMedia_clicked()
{
    static int count = 0;
    QString file(QFileDialog::getOpenFileName(this, "please choose video file", "./", "(*.mp4 *.flv);"));
    if (file.size() == 0)
    {
        return;
    }
    mediaMap.insert(count, file);
    count++;
    QFileInfo fileInfo(file);
    QString fileName = fileInfo.fileName(); // 获取文件名
    // 将文件名添加到list中
    list.append(fileName);
    // 设置model并更新listView
    listModel.setStringList(list);
    ui.listView->setModel(&listModel);
}

void FFMediaPlayer_64::on_listView_clicked(const QModelIndex& index)
{
    media->SetFileName(mediaMap.find(index.row()).value());
    ui.horizontalSlider->setRange(0, media->getDuration());
    Seconds = media->getDuration() % 60;
    Minutes = media->getDuration() / 60;
    char buffer[64] = { 0 };
    sprintf(buffer, "%02d:%02d", Minutes, Seconds);
    ui.Curr->setText(buffer);
    sprintf(buffer, "00:00");
    ui.TotalTime->setText(buffer);
    ui.horizontalSlider->setValue(0);
    Count = 1;
    printf("Finish set file\n");
    fflush(NULL);
    if (media->suppLevel == High)
    {
        media->level = High;
        ui.low->show();
        ui.high->show();
        ui.low->setStyleSheet("QPushButton { background-color: " + QColor(255, 255, 255).name() + "; }");
        ui.high->setStyleSheet("QPushButton { background-color: " + QColor(173, 216, 210).name() + "; }");
    }
    else 
    {
        media->level = Low;
        ui.high->hide();
        ui.low->show();
        ui.low->setStyleSheet("QPushButton { background-color: " + QColor(173, 216, 210).name() + "; }");
    }
}

void FFMediaPlayer_64::on_Play_clicked()
{
    if (media->getFileName().size() == 0) {
        //QMessageBox::information(this, "Warn","cant play");
        return;
    }
    media->Play();
}

void FFMediaPlayer_64::Update()
{
    printf("Update Time: Count: %d\n", Count);
    fflush(NULL);
    ui.horizontalSlider->setValue(Count);
    Seconds = Count % 60;
    Minutes = Count / 60;
    char buffer[64] = { 0 };
    sprintf(buffer, "%02d:%02d", Minutes, Seconds);

    //转化完成后就进行显示
    ui.TotalTime->setText(buffer);
    ui.horizontalSlider->setValue(Count);
    Count++;
}

void FFMediaPlayer_64::on_horizontalSlider_2_valueChanged(int value)
{
    media->setVolume(value);
}

void FFMediaPlayer_64::on_horizontalSlider_sliderPressed()
{
    flag1 = 2;
}

void FFMediaPlayer_64::on_horizontalSlider_sliderReleased()
{
    if (media->getFileName().size() == 0)
    {
        return;
    }
    printf("SeekValue: %d\n", ui.horizontalSlider->value());
    fflush(NULL);
    media->Seek(ui.horizontalSlider->value());
    Count = ui.horizontalSlider->value();
    ui.horizontalSlider->setValue(Count);
    Update();
    Count--;
    flag1 = 1;
}

void FFMediaPlayer_64::on_horizontalSlider_valueChanged(int value)
{
    if (flag2 == 1)
    {
        return;
    }
    printf("value: %d   SetValue: %d\n", value, ui.horizontalSlider->value());
    fflush(NULL);
    if (media->getFileName().size() == 0)
    {
        return;
    }
    this->Count = value;
    media->Seek(value);
    printf("AudioTs: %f\n", media->GetTime());
    fflush(NULL);
    flag2 = 1;
}

void FFMediaPlayer_64::on_horizontalSlider_actionTriggered(int action)
{
    if (flag1 == 2)
    {
        return;
    }
    flag2 = 2;
}

void FFMediaPlayer_64::on_Speed1_clicked()
{
    ui.Speed1->setStyleSheet("QPushButton { background-color: " + QColor(173, 216, 210).name() + "; }");
    ui.Speed1_25->setStyleSheet("QPushButton { background-color: " + QColor(255, 255, 255).name() + "; }");
    media->UpdateSpeed(1);
}


void FFMediaPlayer_64::on_Speed1_25_clicked()
{
    ui.Speed1_25->setStyleSheet("QPushButton { background-color: " + QColor(173, 216, 210).name() + "; }");
    ui.Speed1->setStyleSheet("QPushButton { background-color: " + QColor(255, 255, 255).name() + "; }");
    media->UpdateSpeed(1.25);
}

//切换到高画质
void FFMediaPlayer_64::on_high_clicked()
{
    media->level = High;
    ui.low->setStyleSheet("QPushButton { background-color: " + QColor(255, 255, 255).name() + "; }");
    ui.high->setStyleSheet("QPushButton { background-color: " + QColor(173, 216, 210).name() + "; }");
}

//切换到低画质
void FFMediaPlayer_64::on_low_clicked()
{
    media->level = Low;
    ui.high->setStyleSheet("QPushButton { background-color: " + QColor(255, 255, 255).name() + "; }");
    ui.low->setStyleSheet("QPushButton { background-color: " + QColor(173, 216, 210).name() + "; }");
}

void FFMediaPlayer_64::on_SavePic_clicked()
{
    QFileInfo fileInfo(mediaMap.find(ui.listView->currentIndex().row()).value());
    QString fileName(fileInfo.fileName());
    printf("FileName: %s\n", fileName.toStdString().c_str());
    fflush(NULL);
    media->SavePic(fileName);
}

void FFMediaPlayer_64::on_horizontalSlider_sliderMoved(int position)
{
    int s, m;
    s = position % 60;
    m = position / 60;

    char buffer[64] = { 0 };
    sprintf(buffer, "%02d:%02d", m, s);
    ui.TotalTime->setText(buffer);
}

bool parseRange(const QString& input, int& start, int& end) {
    QStringList parts = input.split('-');

    if (parts.size() == 2) {
        bool startOk, endOk;
        start = parts[0].toInt(&startOk);
        end = parts[1].toInt(&endOk);

        // 确保转换成功
        if (startOk && endOk) {
            return true;
        }
    }
    return false;
}

void FFMediaPlayer_64::Stop()
{
    printf("Recv Signal\n");
    fflush(NULL);
    if (Play == 2)
    {
        Play = 3;
        int rand = QRandomGenerator::global()->bounded(Stop_Time_Begin, Stop_Time_End);
        media->Play();
        printf("Timer start\n");
        fflush(NULL);
        Timer->start(rand * 1000);
        printf("DelayTime: %d\n", rand);
    }
    else
    {
        printf("Timer out\n");
        fflush(NULL);
        Play = -1;
        int rand = QRandomGenerator::global()->bounded(FR_Begin, FR_End);
        int FR = QRandomGenerator::global()->bounded(1, 2);
        if (FR == 1)
        {
            if ((rand + ui.horizontalSlider->value()) >= media->getDuration())
            {
                media->Seek(media->getDuration());
            }
            else
            {
                media->Seek(rand + ui.horizontalSlider->value());
            }
        }
        else
        {
            if ((ui.horizontalSlider->value() - rand) <= 0)
            {
                media->Seek(0);
            }
            else
            {
                media->Seek(ui.horizontalSlider->value() - rand);
            }
        }
    }
    return;
}