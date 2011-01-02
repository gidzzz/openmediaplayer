#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPainter>
#include <QDebug>
#include <QtGui>
#include <musicwindow.h>
#include <videoswindow.h>
#include <internetradiowindow.h>
#ifdef Q_WS_MAEMO_5
#include "mafwrendereradapter.h"
#endif
#include "ui_mainwindow.h"

#define musicIcon "/usr/share/icons/hicolor/164x164/hildon/mediaplayer_main_button_music.png"
#define videosIcon "/usr/share/icons/hicolor/164x164/hildon/mediaplayer_main_button_video.png"
#define radioIcon "/usr/share/icons/hicolor/164x164/hildon/mediaplayer_main_button_radio.png"
#define shuffleIcon "/usr/share/icons/hicolor/164x164/hildon/mediaplayer_main_button_shuffle.png"
#define backgroundImage "/etc/hildon/theme/mediaplayer/background.png"

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
    void setButtonIcons();
    void connectSignals();
    void setLabelText();

private slots:
    void showMusicWindow();
    void showVideosWindow();
    void orientationChanged();
    void showAbout();
    void processListClicks(QListWidgetItem*);
    void showInternetRadioWindow();
};

#endif // MAINWINDOW_H
