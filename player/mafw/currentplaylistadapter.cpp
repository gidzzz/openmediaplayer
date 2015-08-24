#include "currentplaylistadapter.h"

#include "mafwplaylistmanageradapter.h"

CurrentPlaylistAdapter::CurrentPlaylistAdapter(MafwRendererAdapter *renderer, QObject *parent) :
    MafwPlaylistAdapter(NULL, parent),
    renderer(renderer)
{
    connect(renderer, SIGNAL(statusReceived(MafwPlaylist*,uint,MafwPlayState,QString,QString)),
            this, SLOT(onStatusReceived(MafwPlaylist*,uint,MafwPlayState,QString,QString)));
    connect(renderer, SIGNAL(ready()), renderer, SLOT(getStatus()));

    connect(renderer, SIGNAL(playlistChanged(GObject*)), this, SLOT(onPlaylistChanged(GObject*)));
}

bool CurrentPlaylistAdapter::isReady()
{
    return this->playlist;
}

void CurrentPlaylistAdapter::assignAudioPlaylist()
{
    assignPlaylist("FmpAudioPlaylist");
}

void CurrentPlaylistAdapter::assignRadioPlaylist()
{
    assignPlaylist("FmpRadioPlaylist");
}

void CurrentPlaylistAdapter::assignVideoPlaylist()
{
    assignPlaylist("FmpVideoPlaylist");
}

void CurrentPlaylistAdapter::assignPlaylist(const char *playlistName)
{
    if (this->name() != playlistName) {
        this->bind(MAFW_PLAYLIST(MafwPlaylistManagerAdapter::get()->createPlaylist(playlistName)), false);
        renderer->assignPlaylist(this->playlist);
    }
}

void CurrentPlaylistAdapter::onStatusReceived(MafwPlaylist *playlist, uint, MafwPlayState, QString, QString)
{
    disconnect(renderer, SIGNAL(statusReceived(MafwPlaylist*,uint,MafwPlayState,QString,QString)),
               this, SLOT(onStatusReceived(MafwPlaylist*,uint,MafwPlayState,QString,QString)));

    if (playlist) {
        this->bind(playlist);
    } else {
        assignAudioPlaylist();
    }
}

void CurrentPlaylistAdapter::onPlaylistChanged(GObject *playlist)
{
    this->bind(MAFW_PLAYLIST(playlist));

    emit contentsChanged(-1, 0, 0);
}
