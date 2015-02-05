#ifndef MAFWPLAYLISTADAPTER_H
#define MAFWPLAYLISTADAPTER_H

#include <QObject>

#include <libmafw/mafw-playlist.h>

class MafwPlaylistAdapter : public QObject
{
    Q_OBJECT

public:
    MafwPlaylistAdapter(MafwPlaylist *playlist, QObject *parent = NULL);
    MafwPlaylistAdapter(const QString &name, QObject *parent = NULL);
    ~MafwPlaylistAdapter();

public:
    QString name();
    void setName(const QString &name);

    bool isRepeat();
    void setRepeat(bool repeat);

    bool isShuffled();
    void setShuffled(bool shuffled);

    uint size();
    char* item(uint index);
    char** items(uint first, uint last);

    bool insertItem(uint index, const QString &objectId);
    bool insertItems(uint index, const char **objectIds);
    bool appendItem(const QString &objectId);
    bool appendItems(const char **objectIds);
    bool appendItems(MafwPlaylistAdapter *source);
    bool moveItem(uint from, uint to);
    bool removeItem(uint index);
    void clear();

    gpointer getItems(uint first, uint last);
    void cancelQuery(gpointer op);

signals:
    // Exposed signals
    void contentsChanged(uint from, uint removed, uint replaced);
    void itemMoved(uint from, uint to);

    // Exposed callbacks
    void gotItem(QString objectId, GHashTable *metadata, uint index, gpointer op);
    void getItemsComplete(gpointer op);

protected:
    MafwPlaylist *playlist;

    void bind(MafwPlaylist *playlist);

private:
    QList<gpointer> ownedOps;

    // All operations for which a notification should be issued upon completion
    static QList<gpointer> activeOps;

    // Signal handlers
    static void onContentsChanged(MafwPlaylist *, guint from, guint removed, guint replaced, gpointer self);
    static void onItemMoved(MafwPlaylist *, guint from, guint to, gpointer self);

    // Callbacks
    static void onItemReceived(MafwPlaylist *, guint index, const gchar *objectId, GHashTable *metadata, gpointer data);
    static void onGetItemsComplete(gpointer data);

    struct GetItemsData
    {
        MafwPlaylistAdapter *adapter;
        gpointer op;
    };
};

#endif // MAFWPLAYLISTADAPTER_H
