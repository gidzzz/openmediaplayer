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
    mafw_playlist_manager = new MafwPlaylistManagerAdapter(this);
    connect(mafwrenderer, SIGNAL(signalGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*,QString)),
            this, SLOT(onGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*, QString)));
    connect(mafwrenderer, SIGNAL(playlistChanged(GObject*)), this, SLOT(onPlaylistChanged(GObject*)));
    connect(mafwrenderer, SIGNAL(rendererReady()), mafwrenderer, SLOT(getStatus()));
}

void MafwPlaylistAdapter::clear()
{
    if(mafw_playlist)
        mafw_playlist_clear (this->mafw_playlist, &error);
}

bool MafwPlaylistAdapter::isRepeat()
{
    if(mafw_playlist) {
        if (mafw_playlist_get_repeat (this->mafw_playlist))
            return true;
        else
            return false;
    } else
        return false;
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

void MafwPlaylistAdapter::getAllItems()
{
    this->getItems(0, -1);
}

void MafwPlaylistAdapter::getItems(int from, int to)
{
#ifdef DEBUG
    qDebug() << "MafwPlaylistAdapter::getItems";
#endif
    if (mafw_playlist) {
        mafw_playlist_get_items_md (this->mafw_playlist,
                                    from,
                                    to,
                                    MAFW_SOURCE_LIST(MAFW_METADATA_KEY_TITLE,
                                                     MAFW_METADATA_KEY_ALBUM,
                                                     MAFW_METADATA_KEY_ARTIST,
                                                     MAFW_METADATA_KEY_URI,
                                                     MAFW_METADATA_KEY_DURATION),
                                    MafwPlaylistAdapter::get_items_cb,
                                    this, NULL);
    }
#ifdef DEBUG
    qDebug() << "MafwPlaylistAdapter::getItems called successfully";
#endif
}

void MafwPlaylistAdapter::getItemsOf(MafwPlaylist *playlist)
{
    mafw_playlist_get_items_md (playlist,
                                0,
                                -1,
                                MAFW_SOURCE_LIST(MAFW_METADATA_KEY_TITLE,
                                                 MAFW_METADATA_KEY_ALBUM,
                                                 MAFW_METADATA_KEY_ARTIST,
                                                 MAFW_METADATA_KEY_URI,
                                                 MAFW_METADATA_KEY_DURATION),
                                MafwPlaylistAdapter::get_items_cb,
                                this, NULL);
}

void MafwPlaylistAdapter::get_items_cb(MafwPlaylist*,
                                       guint index,
                                       const char *object_id,
                                       GHashTable *metadata,
                                       gpointer user_data)
{
    emit static_cast<MafwPlaylistAdapter*>(user_data)->onGetItems(QString::fromUtf8(object_id), metadata, index);
}

void MafwPlaylistAdapter::onGetStatus(MafwPlaylist* playlist, uint, MafwPlayState, const char*, QString)
{
#ifdef DEBUG
    qDebug() << "MafwPlaylistAdapter::onGetStatus";
#endif
    this->mafw_playlist = playlist;
    if (this->isPlaylistNull()) {
        if (mafwrenderer->isRendererReady())
            this->assignAudioPlaylist();
        else
            connect(mafwrenderer, SIGNAL(rendererReady()), this, SLOT(assignAudioPlaylist()));
    }

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
    mafw_playlist = MAFW_PLAYLIST(mafw_playlist_manager->createPlaylist("FmpAudioPlaylist"));
    mafwrenderer->assignPlaylist(mafw_playlist);
}

void MafwPlaylistAdapter::assignVideoPlaylist()
{
    mafw_playlist = MAFW_PLAYLIST(mafw_playlist_manager->createPlaylist("FmpVideoPlaylist"));
    mafwrenderer->assignPlaylist(mafw_playlist);
}

void MafwPlaylistAdapter::assignRadioPlaylist()
{
    mafw_playlist = MAFW_PLAYLIST(mafw_playlist_manager->createPlaylist("FmpRadioPlaylist"));
    mafwrenderer->assignPlaylist(mafw_playlist);
}

void MafwPlaylistAdapter::duplicatePlaylist(QString newName)
{
    mafw_playlist_manager->duplicatePlaylist(newName, mafw_playlist_manager->createPlaylist(this->playlistName()));
}

bool MafwPlaylistAdapter::isPlaylistNull()
{
    if (mafw_playlist)
        return false;
    else
        return true;
}
