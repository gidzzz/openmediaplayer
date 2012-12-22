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
    void clear(MafwPlaylist *playlist);
    bool isRepeat();
    bool isShuffled();
    bool isPlaylistNull();
    void setRepeat(bool repeat);
    void setShuffled(bool shuffled);
    void insertUri(QString uri, guint index);
    void insertItem(QString objectId, guint index);
    void appendUri(QString url);
    void appendItem(QString objectId);
    void appendItem(MafwPlaylist *playlist, QString objectId);
    void appendItems(const gchar** oid);
    void appendItems(MafwPlaylist *playlist, const gchar** oid);
    void moveItem(int from, int to);
    void removeItem(int index);
    void duplicatePlaylist(QString newName);
    int getSize();
    int getSizeOf(MafwPlaylist *playlist);
    gpointer getItemsOf(MafwPlaylist *playlist);
    gpointer getItemsOf(MafwPlaylist *playlist, int from, int to);
    gpointer getItems(int from, int to);
    gpointer getAllItems();
    QString playlistName();
    MafwPlaylist *mafw_playlist;

    static void get_items_cb(MafwPlaylist*, guint index, const char *object_id, GHashTable *metadata, gpointer);
    static void get_items_free_cbarg(gpointer user_data);
    static void onContentsChanged(MafwPlaylist*, guint from, guint nremove, guint nreplace, gpointer user_data);
    static void onItemMoved(MafwPlaylist*, guint from, guint to, gpointer user_data);


signals:
    void onGetItems(QString object_id, GHashTable *metadata, guint index, gpointer op);
    void getItemsComplete(gpointer op);
    void playlistChanged();
    void contentsChanged(guint from, guint nremove, guint nreplace);
    void itemMoved(guint from, guint to);

public slots:
    void assignAudioPlaylist();
    void assignVideoPlaylist();
    void assignRadioPlaylist();

private:
    void connectPlaylistSignals();
    void disconnectPlaylistSignals();
    gulong contents_changed_handler;
    gulong item_moved_handler;
    MafwRendererAdapter *mafwrenderer;
    GError *error;

private slots:
    void onGetStatus(MafwPlaylist* playlist, uint, MafwPlayState, const char*, QString);
    void onPlaylistChanged(GObject* playlist);
};

struct get_items_cb_payload
{
    MafwPlaylistAdapter* adapter;
    gpointer op;
};

#endif // MAFWPLAYLISTADAPTER_H
