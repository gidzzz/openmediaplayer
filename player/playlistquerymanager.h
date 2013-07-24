#ifndef PLAYLISTQUERYMANAGER_H
#define PLAYLISTQUERYMANAGER_H

#include "includes.h"
#include <QObject>
#include <QList>
#include <QDebug>

#include "mafw/mafwplaylistadapter.h"

class PlaylistQueryManager : public QObject
{
    Q_OBJECT

public:
    explicit PlaylistQueryManager(QObject *parent, MafwPlaylistAdapter *playlist, MafwPlaylist *mafwplaylist = NULL);
    ~PlaylistQueryManager();
    void getItems(int first, int last);
    void itemsInserted(int from, int amount);
    void itemsRemoved(int from, int amount);

signals:
    void onGetItems(QString objectId, GHashTable *metadata, guint index);

public slots:
    void setPriority(int position);

private:
    void mergeRequest(int first, int last);
    void queryPlaylist();
    void restart();

    MafwPlaylistAdapter *playlist;
    MafwPlaylist *mafwplaylist;
    QList<int*> requests;
    gpointer getItemsOp;
    int priority;
    int* rangeInProgress;

private slots:
    void onGetItems(QString objectId, GHashTable *metadata, guint index, gpointer op);
    void onRequestComplete(gpointer op);
};

#endif // PLAYLISTQUERYMANAGER_H
