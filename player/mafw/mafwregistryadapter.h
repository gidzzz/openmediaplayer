#ifndef MAFWREGISTRYADAPTER_H
#define MAFWREGISTRYADAPTER_H

#include <QObject>

#include "mafwrendereradapter.h"
#include "mafwsourceadapter.h"
#include "currentplaylistadapter.h"

class MafwRegistryAdapter : public QObject
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

    static MafwRegistryAdapter* get();

    MafwSource* findSourceByUUID(const QString &uuid);

    MafwRendererAdapter* renderer();
    MafwSourceAdapter* source(RecognizedSource source);
    CurrentPlaylistAdapter* playlist();

    bool isRecognized(const QString &uuid);

signals:
    void sourceAdded(MafwSource *source);
    void sourceAdded(const QString &uuid, const QString &name);
    void sourceRemoved(MafwSource *source);
    void sourceRemoved(const QString &uuid, const QString &name);

private:
    static MafwRegistryAdapter *instance;
    MafwRegistryAdapter();

    MafwRegistry *registry;

    // Reusable adapters
    MafwRendererAdapter *m_renderer;
    MafwSourceAdapter *sources[RecognizedSourceCount];
    CurrentPlaylistAdapter *m_playlist;

    // Signal handlers
    static void onSourceAdded(MafwRegistry *, MafwSource *source, MafwRegistryAdapter *self);
    static void onSourceRemoved(MafwRegistry *, MafwSource *source, MafwRegistryAdapter *self);
};

#endif // MAFWREGISTRYADAPTER_H
