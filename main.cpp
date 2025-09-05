#include "FFMediaPlayer_64.h"
#include "Media.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    FFMediaPlayer_64 w;
    w.show();
    return a.exec();
}
