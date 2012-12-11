#include "playlistquerymanager.h"

#define FIRST 0
#define LAST 1
#define BATCH_SIZE 100
#define ITEM_HEIGHT 70

PlaylistQueryManager::PlaylistQueryManager(QObject *parent, MafwPlaylistAdapter *playlist, MafwPlaylist *mafwplaylist) :
    QObject(parent)
{
    this->playlist = playlist;
    this->mafwplaylist = mafwplaylist;
    priority = 0;
    getItemsOp = NULL;
    connect(playlist, SIGNAL(onGetItems(QString,GHashTable*,guint,gpointer)),
            this, SLOT(onGetItems(QString,GHashTable*,guint,gpointer)));
    connect(playlist, SIGNAL(getItemsComplete(gpointer)),
            this, SLOT(onRequestComplete(gpointer)));
}

PlaylistQueryManager::~PlaylistQueryManager()
{
    if (getItemsOp)
        mafw_playlist_cancel_get_items_md(getItemsOp);

    while (requests.size() > 0)
        delete requests.takeLast();
}

void PlaylistQueryManager::setPriority(int position)
{
    priority = position/ITEM_HEIGHT;
}

void PlaylistQueryManager::getItems(int first, int last)
{
    if (mafwplaylist == NULL) {
        if (last < 0 || last >= playlist->getSize())
        last = playlist->getSize()-1;
    } else {
        if (last < 0 || last >= playlist->getSizeOf(mafwplaylist))
        last = playlist->getSizeOf(mafwplaylist)-1;
    }

    if (last < first)
        return;

    mergeRequest(first, last);

    if (!getItemsOp)
        queryPlaylist();
}

void PlaylistQueryManager::mergeRequest(int first, int last)
{
    int i = 0;

    while (i < requests.size()) {

        // temporary entry
        if (requests.at(i) == rangeInProgress) ++i;

        // full coverage
        else if (first <= requests.at(i)[FIRST] && last >= requests.at(i)[LAST]) {
            delete requests.takeAt(i);
        }

        // on the left side
        else if (first < requests.at(i)[FIRST] && last >= requests.at(i)[FIRST]) {
            last = requests.at(i)[LAST];
            delete requests.takeAt(i);
        }

        // on the right side
        else if (last > requests.at(i)[LAST] && first <= requests.at(i)[LAST]) {
            first = requests.at(i)[FIRST];
            delete requests.takeAt(i);
        }

        // no intersection
        else ++i;
    }

    int *mergedRange = new int[2];
    mergedRange[FIRST] = first;
    mergedRange[LAST] = last;
    requests.append(mergedRange);
}

void PlaylistQueryManager::queryPlaylist()
{
    if (requests.isEmpty()) return;

    // search for a good request to process
    int* best = requests.at(0);
    for (int i = 1; i < requests.size(); i++)
        if (qAbs(priority - (requests.at(i)[FIRST] + requests.at(i)[LAST])/2) < qAbs(priority - (best[FIRST] + best[LAST])/2))
            best = requests.at(i);

    // try to fit a batch in the selected request
    int first = qBound(best[FIRST], priority-(BATCH_SIZE-1)/2, best[LAST]);
    int last = qBound(best[FIRST], priority+BATCH_SIZE/2, best[LAST]);

    // distribute remaining batch budget and update the list of requests
    int pool = BATCH_SIZE - (1+last-first);

    // left side filled
    if (first == best[FIRST]) {
        last = qMin(best[LAST], last+pool);
        if (last == best[LAST]) { // whole range filled
            requests.removeOne(best);
            delete best;
        }
        else // partial fill
            best[FIRST] = last+1;
    }

    // right side filled
    else if (last == best[LAST]) {
        first = qMax(best[FIRST], first-pool);
        if (first == best[FIRST]) { // whole range filled
            requests.removeOne(best);
            delete best;
        }
        else // partial fill
            best[LAST] = first-1;
    }

    // split required
    else {
        int* tail = new int[2];
        tail[FIRST] = last+1;
        tail[LAST] = best[LAST];
        requests.append(tail);

        best[LAST] = first-1;
    }

    if (mafwplaylist == NULL)
        getItemsOp = playlist->getItems(first, last);
    else
        getItemsOp = playlist->getItemsOf(mafwplaylist, first, last);

    qDebug() << first << "-" << last << getItemsOp;

    // throw the range into the list of requests, in case of restart
    rangeInProgress = new int[2];
    rangeInProgress[FIRST] = first;
    rangeInProgress[LAST] = last;
    requests.append(rangeInProgress);
}

void PlaylistQueryManager::onGetItems(QString objectId, GHashTable *metadata, guint index, gpointer op)
{
    if (op != getItemsOp) return;

    emit onGetItems(objectId, metadata, index);
}

void PlaylistQueryManager::onRequestComplete(gpointer op)
{
    if (getItemsOp == op) {
        qDebug() << "query" << op << "complete";
        getItemsOp = NULL;
        requests.removeOne(rangeInProgress);
        queryPlaylist();
    }
}

void PlaylistQueryManager::itemsInserted(int from, int amount)
{
    bool restartRequired = false;
    for (int i = 0; i < requests.size(); i++)

        // to the right, no intersection
        if (requests.at(i)[FIRST] >= from) {
            if (requests.at(i) == rangeInProgress) restartRequired = true;
            requests.at(i)[FIRST] += amount;
            requests.at(i)[LAST] += amount;
        }

        // hit in already existing requst, inflating
        else if (requests.at(i)[LAST] >= from) {
            if (requests.at(i) == rangeInProgress) restartRequired = true;
            requests.at(i)[LAST] += amount;
        }

        // else to the left, no intersection

    if (restartRequired) restart();
}

void PlaylistQueryManager::itemsRemoved(int from, int amount)
{
    bool restartRequired = false;
    for (int i = 0; i < requests.size(); i++)

        // beginning to the right of the beginning of the removed area
        if (requests.at(i)[FIRST] >= from) {
            if (requests.at(i) == rangeInProgress) restartRequired = true;

            // no intersection
            if (requests.at(i)[FIRST] > from+amount) {
                requests.at(i)[FIRST] -= amount;
                requests.at(i)[LAST] -= amount;
            }

            // completly covered
            else if (requests.at(i)[LAST] <= from+amount) {
                delete requests.takeAt(i);
                --i;
            }

            // sticks out on the right
            else {
                requests.at(i)[FIRST] = from;
                requests.at(i)[LAST] -= amount;
            }
        }

        // beginning to the left
        else {

            // sticks out on both sides
            if (requests.at(i)[LAST] >= from+amount) {
                if (requests.at(i) == rangeInProgress) restartRequired = true;
                requests.at(i)[LAST] -= amount;
            }

            // sticks out on the left
            else if (requests.at(i)[LAST] >= from) {
                if (requests.at(i) == rangeInProgress) restartRequired = true;
                requests.at(i)[LAST] = from-1;
            }

            // else no intersection
        }

    if (restartRequired) restart();
}

void PlaylistQueryManager::restart()
{
    qDebug() << "restarting query";
    if (getItemsOp) {
        mafw_playlist_cancel_get_items_md(getItemsOp); // does it call the destructor?
        getItemsOp = NULL;

        int i = requests.indexOf(rangeInProgress);
        if (i != -1) {
            requests.removeAt(i);
            mergeRequest(rangeInProgress[FIRST], rangeInProgress[LAST]);
            delete rangeInProgress;
        }
    }

    queryPlaylist();
}
