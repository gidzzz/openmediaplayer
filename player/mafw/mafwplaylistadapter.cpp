/**************************************************************************
    This file is part of Open MediaPlayer
    Copyright (C) 2010-2011 Mohammad Abu-Garbeyyeh

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

#include "mafwplaylistadapter.h"

MafwPlaylistAdapter::MafwPlaylistAdapter(QObject *parent, MafwRendererAdapter *mra) :
    QObject(parent),
    mafwrenderer(mra)
{
    mafw_playlist = NULL;
    connect(mafwrenderer, SIGNAL(signalGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*,QString)),
            this, SLOT(onGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*, QString)));
    connect(mafwrenderer, SIGNAL(playlistChanged(GObject*)), this, SLOT(onPlaylistChanged(GObject*)));
    connect(mafwrenderer, SIGNAL(rendererReady()), mafwrenderer, SLOT(getStatus()));

#ifdef MAFW_WORKAROUNDS
    // Calling mafw_playlist_get_size() for the first time after restarting the
    // device or after killing MAFW daemons can block execution for a long time
    // (~20 s). However, ensuring that mafw_playlist_manager_get() is called
    // before the blocking function is enough to eliminate the delay.
    // mafw_playlist_manager_get() itself is very fast, so the delay completely
    // disappears rather than moves in time.
    MafwPlaylistManagerAdapter::get();
#endif
}

void MafwPlaylistAdapter::clear()
{
    if (mafw_playlist) {
        // Calling play() after clearing the current playlist in the paused
        // state will cause the renderer to enter the stopped state, but calling
        // play() afterwards will not start the playback. To get out of this,
        // play() followed by resume() is necessary. The situation can be
        // prevented by calling stop() before clearing the playlist.
        mafwrenderer->stop();
        mafw_playlist_clear (this->mafw_playlist, &error);
    }
}

void MafwPlaylistAdapter::clear(MafwPlaylist *playlist)
{
    if(playlist)
        mafw_playlist_clear (playlist, &error);
}

bool MafwPlaylistAdapter::isRepeat()
{
    return mafw_playlist && mafw_playlist_get_repeat(mafw_playlist);
}

void MafwPlaylistAdapter::setRepeat(bool repeat)
{
    if(mafw_playlist)
        mafw_playlist_set_repeat (this->mafw_playlist, repeat);
}

bool MafwPlaylistAdapter::isShuffled()
{
    if(mafw_playlist)
        return mafw_playlist_is_shuffled (this->mafw_playlist);
    else
        return false;
}

void MafwPlaylistAdapter::setShuffled(bool shuffled)
{
    if(mafw_playlist) {
        if(shuffled) {
            mafw_playlist_shuffle (this->mafw_playlist, &error);
        } else {
            mafw_playlist_unshuffle (this->mafw_playlist, &error);
        }
    }
}

void MafwPlaylistAdapter::insertUri(QString uri, guint index)
{
    if(mafw_playlist)
        mafw_playlist_insert_uri (this->mafw_playlist, index, uri.toUtf8(), &error);
}

void MafwPlaylistAdapter::insertItem(QString objectId, guint index)
{
    if(mafw_playlist)
        mafw_playlist_insert_item (this->mafw_playlist, index, objectId.toUtf8(), &error);
}

void MafwPlaylistAdapter::appendUri(QString uri)
{
    if(mafw_playlist)
        mafw_playlist_append_uri (this->mafw_playlist, uri.toUtf8(), &error);
}

void MafwPlaylistAdapter::appendItem(QString objectId)
{
    if(mafw_playlist)
        mafw_playlist_append_item (this->mafw_playlist, objectId.toUtf8(), &error);
}

void MafwPlaylistAdapter::appendItem(MafwPlaylist *playlist, QString objectId)
{
    if(playlist)
        mafw_playlist_append_item (playlist, objectId.toUtf8(), &error);
}

void MafwPlaylistAdapter::appendItems(const gchar** oid)
{
    if(mafw_playlist)
        mafw_playlist_append_items (this->mafw_playlist, oid, &error);
}

void MafwPlaylistAdapter::appendItems(MafwPlaylist *playlist, const gchar** oid)
{
    if(playlist)
        mafw_playlist_append_items (playlist, oid, &error);
}

void MafwPlaylistAdapter::moveItem(int from, int to)
{
    if(mafw_playlist)
        mafw_playlist_move_item (this->mafw_playlist, from, to, &error);
}

void MafwPlaylistAdapter::removeItem(int index)
{
    if(mafw_playlist)
        mafw_playlist_remove_item (this->mafw_playlist, index, &error);
}

int MafwPlaylistAdapter::getSize()
{
    guint size = mafw_playlist_get_size (this->mafw_playlist, &error);
    int sizeAsInt = size;
    return sizeAsInt;
}

int MafwPlaylistAdapter::getSizeOf(MafwPlaylist *playlist)
{
    int size = mafw_playlist_get_size (playlist, &error);
    return size;
}

gpointer MafwPlaylistAdapter::getAllItems()
{
    return this->getItems(0, -1);
}

gpointer MafwPlaylistAdapter::getItems(int from, int to)
{
#ifdef DEBUG
    qDebug() << "MafwPlaylistAdapter::getItems";
#endif

    if (mafw_playlist) {
        get_items_cb_payload* pl = new get_items_cb_payload;
        pl->adapter = this;
        pl->op = mafw_playlist_get_items_md (this->mafw_playlist,
                                             from,
                                             to,
                                             MAFW_SOURCE_LIST(MAFW_METADATA_KEY_TITLE,
                                                              MAFW_METADATA_KEY_ALBUM,
                                                              MAFW_METADATA_KEY_ARTIST,
                                                              MAFW_METADATA_KEY_URI,
                                                              MAFW_METADATA_KEY_DURATION),
                                             MafwPlaylistAdapter::get_items_cb,
                                             pl, get_items_free_cbarg);
        return pl->op;
    } else return NULL;
#ifdef DEBUG
    qDebug() << "MafwPlaylistAdapter::getItems called successfully";
#endif
}

gpointer MafwPlaylistAdapter::getItemsOf(MafwPlaylist *playlist)
{
    get_items_cb_payload* pl = new get_items_cb_payload;
    pl->adapter = this;
    pl->op = mafw_playlist_get_items_md (playlist,
                                         0,
                                         -1,
                                         MAFW_SOURCE_LIST(MAFW_METADATA_KEY_TITLE,
                                                          MAFW_METADATA_KEY_ALBUM,
                                                          MAFW_METADATA_KEY_ARTIST,
                                                          MAFW_METADATA_KEY_URI,
                                                          MAFW_METADATA_KEY_DURATION),
                                         MafwPlaylistAdapter::get_items_cb,
                                         pl, get_items_free_cbarg);
    return pl->op;
}

gpointer MafwPlaylistAdapter::getItemsOf(MafwPlaylist *playlist, int from, int to)
{
    get_items_cb_payload* pl = new get_items_cb_payload;
    pl->adapter = this;
    pl->op = mafw_playlist_get_items_md (playlist,
                                         from,
                                         to,
                                         MAFW_SOURCE_LIST(MAFW_METADATA_KEY_TITLE,
                                                          MAFW_METADATA_KEY_ALBUM,
                                                          MAFW_METADATA_KEY_ARTIST,
                                                          MAFW_METADATA_KEY_URI,
                                                          MAFW_METADATA_KEY_DURATION),
                                         MafwPlaylistAdapter::get_items_cb,
                                         pl, get_items_free_cbarg);
    return pl->op;
}

void MafwPlaylistAdapter::get_items_cb(MafwPlaylist*,
                                       guint index,
                                       const char *object_id,
                                       GHashTable *metadata,
                                       gpointer user_data)
{
    MafwPlaylistAdapter* adapter = static_cast<get_items_cb_payload*>(user_data)->adapter;
                     gpointer op = static_cast<get_items_cb_payload*>(user_data)->op;

    emit adapter->onGetItems(QString::fromUtf8(object_id), metadata, index, op);
}

void MafwPlaylistAdapter::get_items_free_cbarg(gpointer user_data)
{
    MafwPlaylistAdapter* adapter = static_cast<get_items_cb_payload*>(user_data)->adapter;
                     gpointer op = static_cast<get_items_cb_payload*>(user_data)->op;

    emit adapter->getItemsComplete(op);
    delete static_cast<get_items_cb_payload*>(user_data);
}

void MafwPlaylistAdapter::onGetStatus(MafwPlaylist* playlist, uint, MafwPlayState, const char*, QString)
{
#ifdef DEBUG
    qDebug() << "MafwPlaylistAdapter::onGetStatus";
#endif
    this->mafw_playlist = playlist;
    if (!mafw_playlist) {
        if (mafwrenderer->isRendererReady())
            this->assignAudioPlaylist();
        else
            connect(mafwrenderer, SIGNAL(rendererReady()), this, SLOT(assignAudioPlaylist()));
    } else
        connectPlaylistSignals();

    disconnect(mafwrenderer, SIGNAL(signalGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*,QString)),
               this, SLOT(onGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*, QString)));
}

void MafwPlaylistAdapter::onPlaylistChanged(GObject* playlist)
{
#ifdef DEBUG
    qDebug() << "MafwPlaylistAdapter::onPlaylistChanged";
#endif
    this->mafw_playlist = MAFW_PLAYLIST(playlist);
    emit playlistChanged();
}

QString MafwPlaylistAdapter::playlistName()
{
    return QString::fromUtf8(mafw_playlist_get_name (mafw_playlist));
}

void MafwPlaylistAdapter::assignAudioPlaylist()
{
    disconnectPlaylistSignals();
    mafw_playlist = MAFW_PLAYLIST(MafwPlaylistManagerAdapter::get()->createPlaylist("FmpAudioPlaylist"));
    connectPlaylistSignals();
    mafwrenderer->assignPlaylist(mafw_playlist);
}

void MafwPlaylistAdapter::assignVideoPlaylist()
{
    disconnectPlaylistSignals();
    mafw_playlist = MAFW_PLAYLIST(MafwPlaylistManagerAdapter::get()->createPlaylist("FmpVideoPlaylist"));
    connectPlaylistSignals();
    mafwrenderer->assignPlaylist(mafw_playlist);
}

void MafwPlaylistAdapter::assignRadioPlaylist()
{
    disconnectPlaylistSignals();
    mafw_playlist = MAFW_PLAYLIST(MafwPlaylistManagerAdapter::get()->createPlaylist("FmpRadioPlaylist"));
    connectPlaylistSignals();
    mafwrenderer->assignPlaylist(mafw_playlist);
}

void MafwPlaylistAdapter::duplicatePlaylist(QString newName)
{
    MafwPlaylistManagerAdapter *mafw_playlist_manager = MafwPlaylistManagerAdapter::get();
    mafw_playlist_manager->duplicatePlaylist(newName, mafw_playlist_manager->createPlaylist(this->playlistName()));
}

bool MafwPlaylistAdapter::isPlaylistNull()
{
    if (mafw_playlist)
        return false;
    else
        return true;
}

void MafwPlaylistAdapter::connectPlaylistSignals()
{
    contents_changed_handler = g_signal_connect(mafw_playlist,
                               "contents-changed",
                               G_CALLBACK(&onContentsChanged),
                               static_cast<void*>(this));
    item_moved_handler = g_signal_connect(mafw_playlist,
                         "item-moved",
                         G_CALLBACK(&onItemMoved),
                         static_cast<void*>(this));
}

void MafwPlaylistAdapter::disconnectPlaylistSignals()
{
    g_signal_handler_disconnect(mafw_playlist, contents_changed_handler);
    g_signal_handler_disconnect(mafw_playlist, item_moved_handler);
}

void MafwPlaylistAdapter::onContentsChanged(MafwPlaylist*, guint from, guint nremove, guint nreplace, gpointer user_data)
{
    emit static_cast<MafwPlaylistAdapter*>(user_data)->contentsChanged(from, nremove, nreplace);
}

void MafwPlaylistAdapter::onItemMoved(MafwPlaylist*, guint from, guint to, gpointer user_data)
{
    emit static_cast<MafwPlaylistAdapter*>(user_data)->itemMoved(from, to);
}
