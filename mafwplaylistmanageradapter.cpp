#include "mafwplaylistmanageradapter.h"

MafwPlaylistManagerAdapter::MafwPlaylistManagerAdapter(QObject *parent) :
    QObject(parent)
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
