#include "currentplaylistadapter.h"

#include "mafwplaylistmanageradapter.h"

CurrentPlaylistAdapter::CurrentPlaylistAdapter(MafwRendererAdapter *renderer, QObject *parent) :
    MafwPlaylistAdapter(NULL, parent),
    renderer(renderer)
{
    connect(renderer, SIGNAL(signalGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*,QString)),
            this, SLOT(onStatusReceived(MafwPlaylist*,uint,MafwPlayState,const char*,QString)));
    connect(renderer, SIGNAL(rendererReady()), renderer, SLOT(getStatus()));

    connect(renderer, SIGNAL(playlistChanged(GObject*)), this, SLOT(onPlaylistChanged(GObject*)));
}

bool CurrentPlaylistAdapter::isReady()
{
    return this->playlist;
}

void CurrentPlaylistAdapter::assignAudioPlaylist()
{
    this->bind(MAFW_PLAYLIST(MafwPlaylistManagerAdapter::get()->createPlaylist("FmpAudioPlaylist")), false);
    renderer->assignPlaylist(this->playlist);
}

void CurrentPlaylistAdapter::assignRadioPlaylist()
{
    this->bind(MAFW_PLAYLIST(MafwPlaylistManagerAdapter::get()->createPlaylist("FmpRadioPlaylist")), false);
    renderer->assignPlaylist(this->playlist);
}

void CurrentPlaylistAdapter::assignVideoPlaylist()
{
    this->bind(MAFW_PLAYLIST(MafwPlaylistManagerAdapter::get()->createPlaylist("FmpVideoPlaylist")), false);
    renderer->assignPlaylist(this->playlist);
}

void CurrentPlaylistAdapter::onStatusReceived(MafwPlaylist *playlist, uint, MafwPlayState, const char *, QString)
{
    disconnect(renderer, SIGNAL(signalGetStatus(MafwPlaylist*,uint,MafwPlayState,const char*,QString)),
               this, SLOT(onStatusReceived(MafwPlaylist*,uint,MafwPlayState,const char*,QString)));

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
