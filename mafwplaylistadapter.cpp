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
    QObject(parent)
#ifdef MAFW
    ,mafwrenderer(mra)
#endif
{
#ifdef MAFW
    connect(mafwrenderer, SIGNAL(signalGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*,QString)),
            this, SLOT(onGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*, QString)));
    QTimer::singleShot(100, mafwrenderer, SLOT(getStatus()));
#endif
}

void MafwPlaylistAdapter::clear()
{
    if(mafw_playlist)
        mafw_playlist_clear (this->mafw_playlist, &error);
}

bool MafwPlaylistAdapter::isRepeat()
{
    if(mafw_playlist) {
        return mafw_playlist_get_repeat (this->mafw_playlist);
    } else {
        return false;
    }
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

void MafwPlaylistAdapter::getItems()
{
    qDebug() << "MafwPlaylistAdapter::getItems";
    if(mafw_playlist) {
        mafw_playlist_get_items_md (this->mafw_playlist,
                                    0,
                                    -1,
                                    MAFW_SOURCE_LIST(MAFW_METADATA_KEY_TITLE,
                                                     MAFW_METADATA_KEY_ALBUM,
                                                     MAFW_METADATA_KEY_ARTIST,
                                                     MAFW_METADATA_KEY_DURATION),
                                    MafwPlaylistAdapter::get_items_cb,
                                    this, NULL);
    }
    qDebug() << "MafwPlaylistAdapter::getItems called successfully";
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
    this->mafw_playlist = playlist;
    disconnect(mafwrenderer, SIGNAL(signalGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*,QString)),
               this, SLOT(onGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*, QString)));
    this->getItems();
}
