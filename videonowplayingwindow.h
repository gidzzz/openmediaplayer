#ifndef VIDEONOWPLAYINGWINDOW_H
#define VIDEONOWPLAYINGWINDOW_H

#include <QMainWindow>
#include <QIcon>
#include <QTimer>
#include <QtDBus>
#include <QDesktopWidget>
#include <QKeyEvent>
#include <QMessageBox>
#include <QPainter>

#include "ui_videonowplayingwindow.h"
#include "includes.h"

#ifdef Q_WS_MAEMO_5
    #include <QMaemo5InformationBox>
    #include <QSpacerItem>
    #include "share.h"
#endif

#ifdef MAFW
    #include "mafw/mafwadapterfactory.h"
#else
    class MafwRendererAdapter;
    class MafwSourceAdapter;
    class MafwPlaylistAdapter;
#endif


namespace Ui {
    class VideoNowPlayingWindow;
}

class VideoNowPlayingWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit VideoNowPlayingWindow(QWidget *parent = 0, MafwAdapterFactory *mafwFactory = 0);
    ~VideoNowPlayingWindow();
    bool eventFilter(QObject*, QEvent *event);

protected:
    void paintEvent(QPaintEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void keyPressEvent(QKeyEvent *);

private:
    Ui::VideoNowPlayingWindow *ui;
    void setIcons();
    void connectSignals();
    void showOverlay(bool show);
    QTimer *volumeTimer;
    QTimer *positionTimer;
    QString objectIdToPlay;
    bool lazySliders;
    bool reverseTime;
    bool portrait;
    bool isOverlayVisible;
    bool gotInitialState;
#ifdef Q_WS_MAEMO_5
    void setDNDAtom(bool dnd);
#endif
#ifdef MAFW
    MafwAdapterFactory *mafwFactory;
    MafwRendererAdapter* mafwrenderer;
    MafwSourceAdapter *mafwSource;
    int colorkey;
    int mafwState;
    int videoLength;
    int pausedPosition;
    int currentPosition;
    bool errorOccured;
    static QColor colorKey() {return QColor(3, 13, 3);}
#endif

private slots:
    void toggleVolumeSlider();
    void volumeWatcher();
    void orientationChanged();
    void onShareClicked();
    void onDeleteClicked();
    void onVolumeSliderPressed();
    void onVolumeSliderReleased();
    void onPrevButtonClicked();
    void onNextButtonClicked();
#ifdef MAFW
    void onMediaChanged(int, char *objectId);
    void onPropertyChanged(const QDBusMessage &msg);
    void onMetadataChanged(QString metadata, QVariant value);
    void onStateChanged(int state);
    void onGetStatus(MafwPlaylist*, uint index, MafwPlayState, const char* object_id, QString error);
    void onPositionChanged(int position, QString);
    void onSourceMetadataRequested(QString, GHashTable *metadata, QString error);
    void playVideo();
    void onErrorOccured(const QDBusMessage &msg);
    void onShareUriReceived(QString objectId, QString uri);
#endif
#ifdef Q_WS_MAEMO_5
    void onLandscapeMode();
#endif
    void onSliderPressed();
    void onSliderReleased();
    void onSliderMoved(int);
};

#endif // VIDEONOWPLAYINGWINDOW_H
