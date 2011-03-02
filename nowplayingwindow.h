#ifndef NOWPLAYINGWINDOW_H
#define NOWPLAYINGWINDOW_H

#include <QMainWindow>
#include <QtGui>
#include <QtDBus>
#include <QTimer>

#include "mirror.h"
#include "cqgraphicsview.h"
#include "ui_nowplayingwindow.h"
#include "includes.h"
#include "delegates/playlistdelegate.h"

#ifdef Q_WS_MAEMO_5
    #include "mafwrendereradapter.h"
    #include "mafwplaylistadapter.h"
    #include "mafwsourceadapter.h"
    #include "fmtxdialog.h"
#else
    class MafwRendererAdapter;
    class MafwSourceAdapter;
#endif
namespace Ui {
    class NowPlayingWindow;
}

class NowPlayingWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit NowPlayingWindow(QWidget *parent = 0, MafwRendererAdapter* mra = 0, MafwSourceAdapter* msa = 0);
    ~NowPlayingWindow();
    void listSongs(QString);

public slots:
    void onSongSelected(int, int, QString, QString, QString, int);
    void setAlbumImage(QString);

private:
    Ui::NowPlayingWindow *ui;
#ifdef MAFW
    MafwRendererAdapter* mafwrenderer;
    MafwSourceAdapter* mafwTrackerSource;
    MafwPlaylistAdapter *playlist;
    int mafwState;
    void showEvent(QShowEvent *);
#endif
    void setButtonIcons();
    void connectSignals();
    QTimer *volumeTimer;
    QTimer *positionTimer;
    int songDuration;
    QGraphicsScene *albumArtScene;
    mirror *m;
    void keyPressEvent(QKeyEvent *);

private slots:
    void toggleVolumeSlider();
    void showFMTXDialog();
    void toggleList();
#ifdef MAFW
    void onVolumeChanged(const QDBusMessage &msg);
    void stateChanged(int state);
    void onPositionChanged(int, QString);
    void updateProgressBar(int, QString);
    void onGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*,QString);
    void onRendererMetadataRequested(QString, QString, QString, QString object_id, QString);
    void onSourceMetadataRequested(QString, GHashTable*, QString);
    void onGetPlaylistItems(QString object_id, GHashTable *metadata, guint index);
    void setPosition(int);
    void onPlaylistItemActivated(QListWidgetItem*);
#endif
    void metadataChanged(QString name, QVariant value);
    void volumeWatcher();
    void onShuffleButtonPressed();
    void onRepeatButtonPressed();
    void orientationChanged();
    void onNextButtonPressed();
    void onPrevButtonPressed();
    void onSliderPressed();
    void onSliderReleased();
};

#endif // NOWPLAYINGWINDOW_H
