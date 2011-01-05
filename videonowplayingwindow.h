#ifndef VIDEONOWPLAYINGWINDOW_H
#define VIDEONOWPLAYINGWINDOW_H

#include <QMainWindow>
#include <QIcon>
#include <QTimer>
#include <QtDBus>
#include "ui_videonowplayingwindow.h"
#include "ui_nowplayingindicator.h"
#include "includes.h"

namespace Ui {
    class VideoNowPlayingWindow;
}

class VideoNowPlayingWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit VideoNowPlayingWindow(QWidget *parent = 0);
    ~VideoNowPlayingWindow();

private:
    Ui::VideoNowPlayingWindow *ui;
    void setIcons();
    void connectSignals();
    QTimer *volumeTimer;

private slots:
    void toggleVolumeSlider();
    void volumeWatcher();
#ifdef Q_WS_MAEMO_5
    void onVolumeChanged(const QDBusMessage &msg);
#endif
};

#endif // VIDEONOWPLAYINGWINDOW_H
