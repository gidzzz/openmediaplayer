#include "currentplaylistmanager.h"

CurrentPlaylistManager* CurrentPlaylistManager::instance = NULL;

CurrentPlaylistManager* CurrentPlaylistManager::acquire(MafwRegistryAdapter *mafwRegistry)
{
    return instance ? instance : instance = new CurrentPlaylistManager(mafwRegistry);
}

CurrentPlaylistManager::CurrentPlaylistManager(MafwRegistryAdapter *mafwRegistry) :
    playlist(mafwRegistry->playlist()),
    mafwTrackerSource(mafwRegistry->source(MafwRegistryAdapter::Tracker))
{
    browseId = MAFW_SOURCE_INVALID_BROWSE_ID;
    songBufferSize = 0;
    currentToken = 0;
}

// Place a request to append browse results to the playlist
uint CurrentPlaylistManager::appendBrowsed(QString objectId, QString filter, QString sorting, uint limit, bool clear)
{
    // Create and enqueue a job
    Job job = { ++currentToken, objectId, filter, sorting, limit, clear };
    jobs.append(job);

    qDebug() << "Adding job" << jobs.last().token << jobs.last().objectId;

    // Start processing the request if idle
    if (jobs.size() == 1) process();

    return job.token;
}

// Execute the first job in the queue
void CurrentPlaylistManager::process()
{
    qDebug() << "Porcessing job" << jobs.first().token;

    Job job = jobs.first();

    // Prepare to receive the results
    connect(mafwTrackerSource, SIGNAL(browseResult(uint,int,uint,QString,GHashTable*,QString)),
            this, SLOT(onBrowseResult(uint,int,uint,QString,GHashTable*,QString)), Qt::UniqueConnection);

    // Request the results
    browseId = mafwTrackerSource->browse(job.objectId, true,
                                         job.filter.isNull() ? NULL : job.filter.toUtf8().data(),
                                         job.sorting.isNull() ? NULL : job.sorting.toUtf8().data(),
                                         MAFW_SOURCE_LIST(MAFW_METADATA_KEY_MIME),
                                         0, job.limit);
}

// Send a notification and clean up
void CurrentPlaylistManager::finalize()
{
    emit finished(jobs.first().token, songBufferSize);

    qDebug() << "Cleaning up after job" << jobs.first().token;

    // Clean up after the finished job
    jobs.removeFirst();
    browseId = MAFW_SOURCE_INVALID_BROWSE_ID;
    songBufferSize = 0;

    // Continue with the next job
    if (!jobs.isEmpty()) process();
}

void CurrentPlaylistManager::onBrowseResult(uint browseId, int remainingCount, uint index, QString objectId, GHashTable* metadata, QString error)
{
    if (browseId != this->browseId) return;

    if (!error.isEmpty()) qDebug() << error;

    // Check if it's the first result
    if (index == 0) {
        // Check if it's just the obligatory empty result to avoid null items in the playlist
        if (remainingCount == 0 && objectId.isNull()) {
            // There are no items, abort
            disconnect(mafwTrackerSource, SIGNAL(browseResult(uint,int,uint,QString,GHashTable*,QString)),
                   this, SLOT(onBrowseResult(uint,int,uint,QString,GHashTable*,QString)));

            // Clean up and move on
            finalize();
        } else {
            // Prepare the buffer
            songBufferSize = remainingCount+1;
            songBuffer = new gchar*[songBufferSize+1];
            songBuffer[songBufferSize] = NULL;
        }
    }

    // Add the result to the buffer
    songBuffer[index] = qstrdup(objectId.toUtf8());

    // Check if it's the last result
    if (remainingCount == 0) {
        // The connection is no longer needed
        disconnect(mafwTrackerSource, SIGNAL(browseResult(uint,int,uint,QString,GHashTable*,QString)),
                   this, SLOT(onBrowseResult(uint,int,uint,QString,GHashTable*,QString)));

        // MIME is required to determine which playlist should be used
        QString mime = QString::fromUtf8(g_value_get_string(mafw_metadata_first(metadata, MAFW_METADATA_KEY_MIME)));
        qDebug() << "Last item MIME:" << mime;

        // Assign the proper playlist
        if (mime.startsWith("audio")) {
            playlist->assignAudioPlaylist();
        } else {
            playlist->assignVideoPlaylist();
        }

        // Also clear the playlist if ordered to do so
        if (jobs.first().clear)
            playlist->clear();

        // Add songs to the playlist
        playlist->appendItems((const gchar**) songBuffer);

        // Clear the buffer
        for (int i = 0; i < songBufferSize; i++)
            delete[] songBuffer[i];
        delete[] songBuffer;

        // Clean up and move on
        finalize();
   }
}
