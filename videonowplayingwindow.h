#ifndef VIDEONOWPLAYINGWINDOW_H
#define VIDEONOWPLAYINGWINDOW_H

#include <QMainWindow>
#include <QIcon>
#include <QTimer>
#include <QtDBus>
#include "ui_videonowplayingwindow.h"

#define wmCloseIcon "/etc/hildon/theme/images/wmBackIconPressed.png"
#define prevButtonIcon "/etc/hildon/theme/mediaplayer/Back.png"
#define playButtonIcon "/etc/hildon/theme/mediaplayer/Play.png"
#define pauseButtonIcon "/etc/hildon/theme/mediaplayer/Pause.png"
#define nextButtonIcon "/etc/hildon/theme/mediaplayer/Forward.png"
#define shareButtonIcon "/usr/share/icons/hicolor/48x48/hildon/general_share.png"
#define deleteButtonIcon "/usr/share/icons/hicolor/48x48/hildon/general_delete.png"
#define volumeButtonIcon "/usr/share/icons/hicolor/64x64/hildon/mediaplayer_volume.png"

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
