#ifndef MUSICWINDOW_H
#define MUSICWINDOW_H

#include <QMainWindow>
#include <nowplayingwindow.h>
#include <QDir>
#include <QStringList>
#include <QDirIterator>

namespace Ui {
    class MusicWindow;
}

class MusicWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MusicWindow(QWidget *parent = 0);
    ~MusicWindow();

public slots:
    void selectSong();

private:
    Ui::MusicWindow *ui;
    NowPlayingWindow *myNowPlayingWindow;
    void listSongs();
};

#endif // MUSICWINDOW_H
