#include "mafwplaylistmanageradapter.h"

MafwPlaylistManagerAdapter::MafwPlaylistManagerAdapter(QObject *parent) :
    QObject(parent)
{
    this->playlist_manager = mafw_playlist_manager_get();
}
