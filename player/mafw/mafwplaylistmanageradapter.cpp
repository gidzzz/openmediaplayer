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

#include "mafwplaylistmanageradapter.h"

MafwPlaylistManagerAdapter* MafwPlaylistManagerAdapter::instance = NULL;

MafwPlaylistManagerAdapter* MafwPlaylistManagerAdapter::get()
{
    return instance ? instance : instance = new MafwPlaylistManagerAdapter();
}

MafwPlaylistManagerAdapter::MafwPlaylistManagerAdapter()
{
    this->playlist_manager = mafw_playlist_manager_get();
}

MafwProxyPlaylist* MafwPlaylistManagerAdapter::createPlaylist(QString playlistName)
{
    return mafw_playlist_manager_create_playlist (playlist_manager, playlistName.toUtf8(), NULL);
}

void MafwPlaylistManagerAdapter::duplicatePlaylist(QString newPlaylistName, MafwProxyPlaylist *playlist)
{
    if (playlist_manager) {
        mafw_playlist_manager_dup_playlist (playlist_manager, playlist, newPlaylistName.toUtf8(), NULL);
        g_object_unref (playlist);
    }
}

void MafwPlaylistManagerAdapter::importPlaylist(QString playlistUri)
{
    if (playlist_manager) {
        mafw_playlist_manager_import (playlist_manager, playlistUri.toUtf8(), NULL, this->import_cb, this, NULL);
    }
}

void MafwPlaylistManagerAdapter::import_cb(MafwPlaylistManager *,
                                           guint import_id,
                                           MafwProxyPlaylist *playlist,
                                           gpointer user_data,
                                           const GError *)
{
    emit static_cast<MafwPlaylistManagerAdapter*>(user_data)->playlistImported(playlist, import_id);
}

MafwProxyPlaylist* MafwPlaylistManagerAdapter::getPlaylist(guint id)
{
    return mafw_playlist_manager_get_playlist (playlist_manager, id, NULL);
}

GPtrArray* MafwPlaylistManagerAdapter::getPlaylists()
{
    return mafw_playlist_manager_get_playlists (playlist_manager, NULL);
}

GArray* MafwPlaylistManagerAdapter::listPlaylists()
{
    return mafw_playlist_manager_list_playlists (playlist_manager, NULL);
}

void MafwPlaylistManagerAdapter::freeListOfPlaylists(GArray *playlist_list)
{
    mafw_playlist_manager_free_list_of_playlists (playlist_list);
}

void MafwPlaylistManagerAdapter::deletePlaylist(QString playlistName)
{
    mafw_playlist_manager_destroy_playlist (playlist_manager, this->createPlaylist(playlistName), NULL);
}
