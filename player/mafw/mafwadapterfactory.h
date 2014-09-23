#ifndef MAFWADAPTERFACTORY_H
#define MAFWADAPTERFACTORY_H

#include <QObject>

#include "mafwrendereradapter.h"
#include "mafwplaylistadapter.h"
#include "mafwsourceadapter.h"

class MafwAdapterFactory : public QObject
{
    Q_OBJECT

public:
    enum RecognizedSource
    {
        Tracker,
        Radio,
        Upnp,
        RecognizedSourceCount
    };

    static MafwAdapterFactory* get();

    MafwSource* findSourceByUUID(const QString &uuid);

    MafwRendererAdapter* getRenderer();
    MafwPlaylistAdapter* getPlaylist();
    MafwSourceAdapter* getTrackerSource();
    MafwSourceAdapter* getRadioSource();
    MafwSourceAdapter* getUpnpSource();

signals:
    void sourceAdded(MafwSource *source);
    void sourceAdded(const QString &uuid, const QString &name);
    void sourceRemoved(MafwSource *source);
    void sourceRemoved(const QString &uuid, const QString &name);

private:
    static MafwAdapterFactory *instance;
    MafwAdapterFactory();

    MafwRegistry *registry;

    // Reusable adapters
    MafwRendererAdapter *renderer;
    MafwPlaylistAdapter *playlist;
    MafwSourceAdapter *sources[RecognizedSourceCount];

    // Signal handlers
    static void onSourceAdded(MafwRegistry *, MafwSource *source, MafwAdapterFactory *self);
    static void onSourceRemoved(MafwRegistry *, MafwSource *source, MafwAdapterFactory *self);
};

#endif // MAFWADAPTERFACTORY_H
