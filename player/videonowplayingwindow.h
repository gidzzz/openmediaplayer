#ifndef VIDEONOWPLAYINGWINDOW_H
#define VIDEONOWPLAYINGWINDOW_H

#include <QMainWindow>
#include <QIcon>
#include <QTimer>
#include <QtDBus>
#include <QKeyEvent>
#include <QMessageBox>
#include <QPainter>

#include "ui_videonowplayingwindow.h"
#include "includes.h"
#include "confirmdialog.h"
#include "rotator.h"
#include "radionowplayingwindow.h"
#include "bookmarkdialog.h"

#ifdef Q_WS_MAEMO_5
    #include <QtGui/QX11Info>
    #include <X11/Xlib.h>
    #include <X11/Xatom.h>
    #include <X11/Xutil.h>
    #include <QMaemo5InformationBox>
    #include <QSpacerItem>
    #include "maemo5deviceevents.h"
    #include "sharedialog.h"
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
    explicit VideoNowPlayingWindow(QWidget *parent = 0, MafwAdapterFactory *mafwFactory = 0, bool overlay = false);
    ~VideoNowPlayingWindow();
    bool eventFilter(QObject*, QEvent *event);

    void play();

protected:
    void mouseReleaseEvent(QMouseEvent *);
    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);
    void changeEvent(QEvent *e);

private:
    Ui::VideoNowPlayingWindow *ui;
    void setIcons();
    void connectSignals();
    void startPositionTimer();
    void showOverlay(bool show);

    QTimer *volumeTimer;
    QTimer *positionTimer;
    QTimer *keyRepeatTimer;

    QString currentObjectId;
    QString playedObjectId;

    QString uri;
    Rotator::Orientation rotatorPolicy;
    bool lazySliders;
    bool reverseTime;
    bool showSettings;
    bool fitToScreen;
    bool portrait;
    bool overlayVisible;
    bool overlayRequestedByUser;
    bool playWhenReady;
    bool saveStateOnClose;
    bool gotInitialStopState;
    bool gotInitialPlayState;
    bool gotCurrentPlayState;
    bool buttonWasDown;
    int keyToRepeat;
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
    int resumePosition;
    int currentPosition;
    int videoWidth;
    int videoHeight;
    static QColor colorKey() { return QColor(3, 13, 3); }
#endif

private slots:
    void setFitToScreen(bool enable);
    void setContinuousPlayback(bool enable);
    void toggleOverlay();
    void toggleSettings();
    void toggleVolumeSlider();
    void onBookmarkClicked();
    void onShareClicked();
    void onDeleteClicked();
    void onVolumeSliderPressed();
    void onVolumeSliderReleased();
    void onPrevButtonClicked();
    void onNextButtonClicked();
    void onScreenLocked(bool locked);
    void repeatKey();
#ifdef MAFW
    void togglePlayback();
    void slowFwd();
    void slowRev();
    void fastFwd();
    void fastRev();
    void onMediaChanged(int, char *objectId);
    void onPropertyChanged(const QDBusMessage &msg);
    void onMetadataChanged(QString name, QVariant value);
    void onStateChanged(int state);
    void onGetStatus(MafwPlaylist*, uint index, MafwPlayState, const char* objectId, QString error);
    void onBufferingInfo(float status);
    void onPositionChanged(int position, QString);
    void handleRendererMetadata(GHashTable *metadata, QString, QString error);
    void handleSourceMetadata(QString objectId, GHashTable *metadata, QString error);
    void onErrorOccured(const QDBusMessage &msg);
#endif
    void onPositionSliderPressed();
    void onPositionSliderReleased();
    void onPositionSliderMoved(int);
};

#endif // VIDEONOWPLAYINGWINDOW_H
