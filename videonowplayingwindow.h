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

protected:
    void paintEvent(QPaintEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void keyPressEvent(QKeyEvent *e);

private:
    Ui::VideoNowPlayingWindow *ui;
    void setIcons();
    void connectSignals();
    void showOverlay(bool show);
    void setAspectRatio(float ratio);
    QTimer *volumeTimer;
    QTimer *positionTimer;
    QString currentObjectId;
    QString uri;
    Rotator::Orientation savedPolicy;
    bool lazySliders;
    bool reverseTime;
    bool showSettings;
    bool fitToScreen;
    bool portrait;
    bool overlayVisible;
    bool overlayRequestedByUser;
    bool saveStateOnClose;
    bool gotInitialState;
    bool buttonWasDown;
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
    int videoWidth;
    int videoHeight;
    bool errorOccured;
    static QColor colorKey() {return QColor(3, 13, 3);}
#endif

private slots:
    void setFitToScreen(bool enable);
    void toggleOverlay();
    void toggleSettings();
    void toggleVolumeSlider();
    void volumeWatcher();
    void orientationChanged(int w, int h);
    void onBookmarkClicked();
    void onShareClicked();
    void onDeleteClicked();
    void onVolumeSliderPressed();
    void onVolumeSliderReleased();
    void onPrevButtonClicked();
    void onNextButtonClicked();
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
    void onGetStatus(MafwPlaylist*, uint index, MafwPlayState, const char* object_id, QString error);
    void onBufferingInfo(float status);
    void onPositionChanged(int position, QString);
    void onRendererMetadataRequested(GHashTable *metadata, QString, QString error);
    void onSourceMetadataRequested(QString objectId, GHashTable *metadata, QString error);
    void onErrorOccured(const QDBusMessage &msg);
    void onShareUriReceived(QString objectId, QString uri);
#endif
    void onSliderPressed();
    void onSliderReleased();
    void onSliderMoved(int);
};

#endif // VIDEONOWPLAYINGWINDOW_H
