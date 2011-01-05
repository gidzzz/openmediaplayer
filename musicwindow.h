#ifndef MUSICWINDOW_H
#define MUSICWINDOW_H

#include <QMainWindow>
#include <nowplayingwindow.h>
#include <QDir>
#include <QStringList>
#include <QDirIterator>
#include <QMenu>
#include <QtGui>
#include <share.h>
#include "songlistitemdelegate.h"
#include "artistlistitemdelegate.h"
#include "ui_musicwindow.h"
#include "includes.h"
#ifdef Q_WS_MAEMO_5
    #include <QMaemo5ValueButton>
    #include "mafwrendereradapter.h"
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
    QMenu *contextMenu;
    MafwRendererAdapter* mafwrenderer;
#ifdef Q_WS_MAEMO_5
    QMaemo5ValueButton *shuffleAllButton;
#else
    QPushButton *shuffleAllButton;
#endif
    void listSongs();
    void connectSignals();
    void populateMenuBar();
    void hideLayoutContents();
    void saveViewState(QVariant);
    void loadViewState();

private slots:
    void onContextMenuRequested(const QPoint&);
    void onShareClicked();
    void orientationChanged();
    void showAlbumView();
    void showPlayListView();
    void showArtistView();
    void showSongsView();
    void showGenresView();
};

#endif // MUSICWINDOW_H
