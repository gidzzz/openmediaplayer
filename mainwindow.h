#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPainter>
#include <QDebug>
#include <QtGui>

#include "musicwindow.h"
#include "videoswindow.h"
#include "internetradiowindow.h"
#include "ui_mainwindow.h"
#include "nowplayingindicator.h"
#include "includes.h"
#include "settingsdialog.h"

#ifdef Q_WS_MAEMO_5
    #include <QMaemo5InformationBox>
#endif
#ifdef MAFW
    #include "mafwrendereradapter.h"
    #include "mafwplaylistadapter.h"
    #include "mafwsourceadapter.h"
    #include <libmafw/mafw-source.h>
#endif

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    MusicWindow *myMusicWindow;
    VideosWindow *myVideosWindow;
    InternetRadioWindow *myInternetRadioWindow;
    void paintEvent(QPaintEvent*);
    void focusInEvent(QFocusEvent *);
    void focusOutEvent(QFocusEvent *);
    void changeEvent(QEvent *);
    void closeEvent(QCloseEvent *);
    bool shuffleNowPlayingWindowCreated;

    void setButtonIcons();
    void connectSignals();
    void setLabelText();
#ifdef MAFW
    MafwSourceAdapter *mafwTrackerSource;
    MafwSourceAdapter *mafwRadioSource;
    MafwRendererAdapter* mafwrenderer;
    MafwPlaylistAdapter* playlist;
    uint browseAllSongsId;
    const char* TAGSOURCE_AUDIO_PATH;
    const char* TAGSOURCE_VIDEO_PATH;
    const char* RADIOSOURCE_PATH;
    void countSongs();
    void countVideos();
    void countRadioStations();
#endif

private slots:
    void orientationChanged();
    void showAbout();
    void processListClicks(QListWidgetItem*);
    void openSettings();
    void showVideosWindow();
    void showInternetRadioWindow();
    void onShuffleAllClicked();
#ifdef MAFW
    void trackerSourceReady();
    void radioSourceReady();
    void countAudioVideoResult(QString objectId, GHashTable* metadata, QString error);
    void countRadioResult(QString objectId, GHashTable* metadata, QString error);
    void browseAllSongs(uint browseId, int remainingCount, uint, QString objectId, GHashTable*, QString);
#endif
};

#endif // MAINWINDOW_H
