#ifndef MAFWPLAYLISTADAPTER_H
#define MAFWPLAYLISTADAPTER_H

#include <QObject>
#include <QString>
#include <QTimer>
#include <QDebug>
#include <libmafw/mafw-playlist.h>
#include "mafwrendereradapter.h"
#include "mafwplaylistmanageradapter.h"

class MafwPlaylistAdapter : public QObject
{
    Q_OBJECT
public:
    explicit MafwPlaylistAdapter(QObject *parent = 0, MafwRendererAdapter* mra = 0);
    void clear();
    bool isRepeat();
    bool isShuffled();
    bool isPlaylistNull();
    void setRepeat(bool repeat);
    void setShuffled(bool shuffled);
    void insertUri(QString uri, guint index);
    void insertItem(QString objectId, guint index);
    void appendUri(QString url);
    void appendItem(QString objectId);
    void assignAudioPlaylist();

    static void get_items_cb(MafwPlaylist*, guint index, const char *object_id, GHashTable *metadata, gpointer);


signals:
    void onGetItems(QString object_id, GHashTable *metadata, guint index);
    void playlistChanged();

public slots:
    void getItems();

private:
    MafwPlaylist *mafw_playlist;
    MafwPlaylistManagerAdapter *mafw_playlist_manager;
    MafwRendererAdapter *mafwrenderer;
    GError *error;

private slots:
    void onGetStatus(MafwPlaylist* playlist, uint, MafwPlayState, const char*, QString);
    void onPlaylistChanged(GObject* playlist);
};

#endif // MAFWPLAYLISTADAPTER_H
