#ifndef MUSICWINDOW_H
#define MUSICWINDOW_H

#include <QMainWindow>
#include <nowplayingwindow.h>
#include <QDir>
#include <QStringList>
#include <QDirIterator>
#ifdef Q_WS_MAEMO_5
#include <QMaemo5ValueButton>
#endif

class MafwRendererAdapter;
namespace Ui {
    class MusicWindow;
}

class MusicWindow : public QMainWindow
{
    Q_OBJECT

public:
  explicit MusicWindow(QWidget *parent = 0, MafwRendererAdapter* mra = 0);
    ~MusicWindow();

public slots:
    void selectSong();

private:
    Ui::MusicWindow *ui;
    NowPlayingWindow *myNowPlayingWindow;
    MafwRendererAdapter* mafwrenderer;
#ifdef Q_WS_MAEMO_5
    QMaemo5ValueButton *shuffleAllButton;
#else
    QPushButton *shuffleAllButton;
#endif
    void listSongs();
};

#endif // MUSICWINDOW_H
