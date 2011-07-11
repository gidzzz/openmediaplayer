#ifndef ENTERTAINMENTVIEW_H
#define ENTERTAINMENTVIEW_H

#include <QMainWindow>
#include <QDeclarativeView>
#include <QGraphicsObject>
#include <QGLWidget>
#include <QTimer>
#include <QListWidgetItem>

#include "ui_entertainmentview.h"
#include "includes.h"

#ifdef MAFW
    #include "mafw/mafwadapterfactory.h"
#endif

namespace Ui {
    class EntertainmentView;
}

class EntertainmentView : public QMainWindow
{
    Q_OBJECT

public:
    explicit EntertainmentView(QWidget *parent = 0, MafwAdapterFactory *mafwFactory = 0);
    ~EntertainmentView();
    void setMetadata(QString songName, QString albumName, QString artistName, QString albumArtUri, int duration);
    void addItemToPlaylist(QListWidgetItem *item);
    void setCurrentRow(int);

signals:
    void titleChanged(QVariant);
    void albumChanged(QVariant);
    void artistChanged(QVariant);
    void albumArtChanged(QVariant);
    void positionChanged(QVariant);
    void durationChanged(QVariant);
    void durationTextChanged(QVariant);
    void stateIconChanged(QVariant);
    void addToPlaylist(QVariant, QVariant, QVariant, QVariant);
    void rowChanged(QVariant);

private:
    Ui::EntertainmentView *ui;
    QVariant title;
    QVariant album;
    QVariant artist;
    QVariant albumArt;
    QVariant duration;
    int songDuration;
    int currentPosition;
    QTimer *positionTimer;
    QObject *rootObject;
#ifdef Q_WS_MAEMO_5
    void setDNDAtom(bool dnd);
#endif
#ifdef MAFW
    MafwAdapterFactory *mafwFactory;
    MafwRendererAdapter* mafwrenderer;
    MafwSourceAdapter *mafwTrackerSource;
    MafwPlaylistAdapter* playlist;
    int mafwState;
#endif

private slots:
#ifdef MAFW
    void onPositionChanged(int position, QString);
    void onGetStatus(MafwPlaylist*,uint,MafwPlayState state,const char*,QString);
    void stateChanged(int state);
    void onPlayClicked();
    void onSliderValueChanged(int position);
    void onPlaylistItemChanged(int);
#endif
};

#endif // ENTERTAINMENTVIEW_H
