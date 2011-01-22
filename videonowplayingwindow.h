#ifndef VIDEONOWPLAYINGWINDOW_H
#define VIDEONOWPLAYINGWINDOW_H

#include <QMainWindow>
#include <QIcon>
#include <QTimer>
#include <QtDBus>
#include <QDesktopWidget>

#include "ui_videonowplayingwindow.h"
#include "includes.h"
#include "qmaemo5rotator.h"

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
    bool portrait;
#ifdef Q_WS_MAEMO_5
    QMaemo5Rotator *rotator;
#endif

private slots:
    void toggleVolumeSlider();
    void volumeWatcher();
    void orientationChanged();
#ifdef Q_WS_MAEMO_5
    void onVolumeChanged(const QDBusMessage &msg);
    void onPortraitMode();
    void onLandscapeMode();
#endif
};

#endif // VIDEONOWPLAYINGWINDOW_H
