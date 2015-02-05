#include "mafwplaylistadapter.h"

#include <libmafw-shared/mafw-shared.h>

#include "mafwplaylistmanageradapter.h"

QList<gpointer> MafwPlaylistAdapter::activeOps;

MafwPlaylistAdapter::MafwPlaylistAdapter(MafwPlaylist *playlist, QObject *parent) :
    QObject(parent),
    playlist(NULL)
{
    bind(playlist);
}

MafwPlaylistAdapter::MafwPlaylistAdapter(const QString &name, QObject *parent) :
    QObject(parent),
    playlist(NULL)
{
    bind(MAFW_PLAYLIST(MafwPlaylistManagerAdapter::get()->createPlaylist(name)));

    // Correction for the reference count increased by bind()
    if (playlist)
        g_object_unref(playlist);
}

MafwPlaylistAdapter::~MafwPlaylistAdapter()
{
    bind(NULL);
}

QString MafwPlaylistAdapter::name()
{
    if (!playlist) return QString();

    gchar *gname = mafw_playlist_get_name(playlist);
    QString qname = QString::fromUtf8(gname);
    g_free(gname);
    return qname;
}

void MafwPlaylistAdapter::setName(const QString &name)
{
    if (playlist)
        mafw_playlist_set_name(playlist, name.toUtf8());
}

bool MafwPlaylistAdapter::isRepeat()
{
    return playlist && mafw_playlist_get_repeat(playlist);
}

void MafwPlaylistAdapter::setRepeat(bool repeat)
{
    if (playlist)
        mafw_playlist_set_repeat(playlist, repeat);
}

bool MafwPlaylistAdapter::isShuffled()
{
    return playlist && mafw_playlist_is_shuffled(playlist);
}

void MafwPlaylistAdapter::setShuffled(bool shuffled)
{
    if (playlist)
        (shuffled ? mafw_playlist_shuffle : mafw_playlist_unshuffle)(playlist, NULL);
}

uint MafwPlaylistAdapter::size()
{
    return playlist ? mafw_playlist_get_size(playlist, NULL) : 0;
}

// NOTE: The result should be freed by the caller using g_free()
char* MafwPlaylistAdapter::item(uint index)
{
    return playlist ? mafw_playlist_get_item(playlist, index, NULL) : NULL;
}

// NOTE: The result should be freed by the caller using g_strfreev()
char** MafwPlaylistAdapter::items(uint first, uint last)
{
    return playlist ? mafw_playlist_get_items(playlist, first, last,  NULL) : NULL;
}

bool MafwPlaylistAdapter::insertItem(uint index, const QString &objectId)
{
    return playlist && mafw_playlist_insert_item(playlist, index, objectId.toUtf8(), NULL);
}

bool MafwPlaylistAdapter::insertItems(uint index, const char **objectIds)
{
    return playlist && mafw_playlist_insert_items(playlist, index, objectIds, NULL);
}

bool MafwPlaylistAdapter::appendItem(const QString &objectId)
{
    return playlist && mafw_playlist_append_item(playlist, objectId.toUtf8(), NULL);
}

bool MafwPlaylistAdapter::appendItems(const char **objectIds)
{
    return playlist && mafw_playlist_append_items(playlist, objectIds, NULL);
}

bool MafwPlaylistAdapter::appendItems(MafwPlaylistAdapter *source)
{
    if (!playlist) return false;

    gchar **items = source->items(0, source->size()-1);
    bool result = appendItems((const gchar**) items);
    g_strfreev(items);
    return result;
}

bool MafwPlaylistAdapter::moveItem(uint from, uint to)
{
    return playlist && mafw_playlist_move_item(playlist, from, to, NULL);
}

bool MafwPlaylistAdapter::removeItem(uint index)
{
    return playlist && mafw_playlist_remove_item(playlist, index, NULL);
}

void MafwPlaylistAdapter::clear()
{
    if (playlist)
        mafw_playlist_clear(playlist, NULL);
}

gpointer MafwPlaylistAdapter::getItems(uint first, uint last)
{
    if (!playlist) return NULL;

    GetItemsData *data = new GetItemsData();
    data->adapter = this;
    data->op = mafw_playlist_get_items_md(playlist, first, last,
                                          MAFW_SOURCE_LIST(MAFW_METADATA_KEY_TITLE,
                                                           MAFW_METADATA_KEY_ALBUM,
                                                           MAFW_METADATA_KEY_ARTIST,
                                                           MAFW_METADATA_KEY_DURATION),
                                          &onItemReceived, data, &onGetItemsComplete);

    activeOps.append(data->op);
    ownedOps.append(data->op);
    return data->op;
}

void MafwPlaylistAdapter::cancelQuery(gpointer op)
{
    if (ownedOps.removeOne(op)) {
        activeOps.removeOne(op);
        mafw_playlist_cancel_get_items_md(op);
    }
}

void MafwPlaylistAdapter::bind(MafwPlaylist *playlist)
{
    // Check if there is anything to do
    if (playlist == this->playlist)
        return;

    if (playlist) {
        // Unbind current playlist, if set, before proceeding
        bind(NULL);

        // Bind
        g_object_ref(playlist);
        g_signal_connect(playlist, "contents-changed", G_CALLBACK(&onContentsChanged), static_cast<void*>(this));
        g_signal_connect(playlist, "item-moved"      , G_CALLBACK(&onItemMoved)      , static_cast<void*>(this));

        this->playlist = playlist;
    } else {
        // Prevent outdated callbacks
        foreach (gpointer op, ownedOps) {
            activeOps.removeOne(op);
            mafw_playlist_cancel_get_items_md(op);
            emit getItemsComplete(op);
        }
        ownedOps.clear();

        // Unbind
        g_signal_handlers_disconnect_matched(this->playlist, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, this);
        g_object_unref(this->playlist);

        this->playlist = NULL;
    }
}

//--- Signal handlers ----------------------------------------------------------

void MafwPlaylistAdapter::onContentsChanged(MafwPlaylist *, guint from, guint removed, guint replaced, gpointer self)
{
    emit static_cast<MafwPlaylistAdapter*>(self)->contentsChanged(from, removed, replaced);
}

void MafwPlaylistAdapter::onItemMoved(MafwPlaylist *, guint from, guint to, gpointer self)
{
    emit static_cast<MafwPlaylistAdapter*>(self)->itemMoved(from, to);
}

//--- Callbacks ----------------------------------------------------------------

void MafwPlaylistAdapter::onItemReceived(MafwPlaylist *, guint index, const gchar *objectId, GHashTable *metadata, gpointer data)
{
    MafwPlaylistAdapter *adapter = static_cast<GetItemsData*>(data)->adapter;
                     gpointer op = static_cast<GetItemsData*>(data)->op;

    emit adapter->gotItem(QString::fromUtf8(objectId), metadata, index, op);
}

void MafwPlaylistAdapter::onGetItemsComplete(gpointer data)
{
    gpointer op = static_cast<GetItemsData*>(data)->op;

    if (activeOps.removeOne(op)) {
        MafwPlaylistAdapter *adapter = static_cast<GetItemsData*>(data)->adapter;
        adapter->ownedOps.removeOne(op);
        emit adapter->getItemsComplete(op);
    }

    delete static_cast<GetItemsData*>(data);
}
