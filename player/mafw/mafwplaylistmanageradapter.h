#ifndef MAFWPLAYLISTMANAGERADAPTER_H
#define MAFWPLAYLISTMANAGERADAPTER_H

#include <QObject>
#include <libmafw-shared/mafw-playlist-manager.h>

class MafwPlaylistManagerAdapter : public QObject
{
    Q_OBJECT
public:
    static MafwPlaylistManagerAdapter* get();

    void importPlaylist(QString playlistUri);
    void duplicatePlaylist(QString newPlaylistName, MafwProxyPlaylist *playlist);
    MafwProxyPlaylist* createPlaylist(QString playlistName);
    MafwProxyPlaylist* getPlaylist(guint id);
    GPtrArray* getPlaylists();
    GArray* listPlaylists();
    void freeListOfPlaylists(GArray *playlist_list);
    void deletePlaylist(QString playlistName);
    static void import_cb(MafwPlaylistManager *, guint import_id, MafwProxyPlaylist *playlist, gpointer user_data, const GError *);

signals:
    void playlistImported(MafwProxyPlaylist *playlist, guint import_id);

public slots:

private:
    explicit MafwPlaylistManagerAdapter();
    static MafwPlaylistManagerAdapter* instance;

    MafwPlaylistManager* playlist_manager;
};

#endif // MAFWPLAYLISTMANAGERADAPTER_H
