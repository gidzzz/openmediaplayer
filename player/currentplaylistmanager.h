#ifndef CURRENTPLAYLISTMANAGER_H
#define CURRENTPLAYLISTMANAGER_H

#include <QDebug>

#include <QObject>
#include <QList>

#include "mafw/mafwadapterfactory.h"

struct Job
{
    uint token;       // operation identifier
    QString objectId; // MAFW object identifier
    QString filter;   // filter string
    QString sorting;  // sort criteria
    uint limit;       // maximal number of results
};

class CurrentPlaylistManager : public QObject
{
    Q_OBJECT

public:
    static CurrentPlaylistManager* acquire(MafwAdapterFactory *factory);

    uint appendBrowsed(QString objectId,
                       QString filter = QString(),
                       QString sorting = QString(),
                       uint limit = MAFW_SOURCE_BROWSE_ALL);

signals:
    void finished(uint browseId, int count);

private:
    static CurrentPlaylistManager *instance;
    CurrentPlaylistManager(MafwAdapterFactory *factory);

    QList<Job> jobs;

    void process();
    void finalize();

    uint currentToken;
    uint browseId;

    int songBufferSize;
    gchar** songBuffer;

    MafwPlaylistAdapter *playlist;
    MafwSourceAdapter *mafwTrackerSource;

private slots:
    void onBrowseResult(uint browseId, int remainingCount, uint index, QString objectId, GHashTable *metadata, QString error);
};

#endif // CURRENTPLAYLISTMANAGER_H
