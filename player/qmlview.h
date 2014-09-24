#ifndef QMLVIEW_H
#define QMLVIEW_H

#include <QMainWindow>
#include <QDeclarativeView>
#include <QGraphicsObject>
#include <QGLWidget>
#include <QTimer>
#include <QListWidgetItem>

#include "ui_qmlview.h"
#include "includes.h"
#include "rotator.h"

#ifdef MAFW
    #include "mafw/mafwregistryadapter.h"
#endif

namespace Ui {
    class QmlView;
}

class QmlView : public QMainWindow
{
    Q_OBJECT

public:
    explicit QmlView(QUrl source, QWidget *parent = 0, MafwRegistryAdapter *mafwRegistry = 0);
    ~QmlView();
    void setMetadata(QString songName, QString albumName, QString artistName, QString albumArtUri, int duration);
    void appendPlaylistItem(QListWidgetItem *item);
    void insertPlaylistItem(int index, QListWidgetItem *item);
    void setPlaylistItem(int index, QListWidgetItem *item);
    void removePlaylistItem(int index);
    void clearPlaylist();
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
    void rowChanged(QVariant);

    void playlistItemAppended(QVariant, QVariant, QVariant);
    void playlistItemInserted(QVariant, QVariant, QVariant, QVariant);
    void playlistItemSet(QVariant, QVariant, QVariant, QVariant);
    void playlistItemRemoved(QVariant);
    void playlistCleared();

private:
    Ui::QmlView *ui;
    QVariant title;
    QVariant album;
    QVariant artist;
    QVariant albumArt;
    QVariant duration;
    int songDuration;
    int currentPosition;
    Rotator::Orientation savedPolicy;
    QTimer *positionTimer;
    QObject *rootObject;
#ifdef Q_WS_MAEMO_5
    void setDNDAtom(bool dnd);
#endif
#ifdef MAFW
    MafwRegistryAdapter *mafwRegistry;
    MafwRendererAdapter* mafwrenderer;
    int mafwState;
#endif

private slots:
#ifdef MAFW
    void onPositionChanged(int position, QString);
    void onGetStatus(MafwPlaylist*,uint,MafwPlayState state,const char*,QString);
    void onStateChanged(int state);
    void onPlayClicked();
    void onSliderValueChanged(int position);
    void onPlaylistItemChanged(int);
#endif
};

#endif // QMLVIEW_H
