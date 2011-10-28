#ifndef MAFWADAPTERFACTORY_H
#define MAFWADAPTERFACTORY_H

#include <QObject>
#include "mafwrendereradapter.h"
#include "mafwsourceadapter.h"
#include "mafwplaylistadapter.h"

class MafwAdapterFactory : public QObject
{
    Q_OBJECT
public:
    explicit MafwAdapterFactory(QObject *parent = 0);
    MafwRendererAdapter *getRenderer();
    MafwSourceAdapter *getTrackerSource();
    MafwSourceAdapter *getRadioSource();
    MafwSourceAdapter *getUpnpSource();
    MafwPlaylistAdapter *getPlaylistAdapter();
    int mafwState();

signals:

public slots:

private:
    MafwRendererAdapter *mafwrenderer;
    MafwSourceAdapter *mafwTrackerSource;
    MafwSourceAdapter *mafwRadioSource;
    MafwSourceAdapter *mafwUpnpSource;
    MafwPlaylistAdapter *playlist;
    MafwPlaylistManagerAdapter *mafw_playlist_manager;
    int state;

private slots:
    void onStateChanged(int state);
    void onGetStatus(MafwPlaylist*, uint, MafwPlayState state, const char *, QString);
};

#endif // MAFWADAPTERFACTORY_H
