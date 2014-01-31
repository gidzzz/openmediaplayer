#ifndef MISSIONCONTROL_H
#define MISSIONCONTROL_H

#include <QObject>

#include "lyricsmanager.h"
#include "metadatawatcher.h"
#include "sleeper.h"

#include "mafw/mafwadapterfactory.h"

class MissionControl : public QObject
{
    Q_OBJECT

public:
    static MissionControl* acquire();

    void setFactory(MafwAdapterFactory *factory);
    void reloadSettings();

    LyricsManager *lyricsManager();
    MetadataWatcher *metadataWatcher();
    Sleeper* sleeper();

private:
    static MissionControl *instance;

    MafwAdapterFactory *mafwFactory;

    MetadataWatcher *m_metadataWatcher;
    LyricsManager *m_lyricsManager;
    Sleeper* m_sleeper;

    QString currentArtist;
    QString currentTitle;

    MissionControl();

private slots:
    void onMediaChanged();
    void onMetadataReady();
    void onMetadataChanged(QString key, QVariant value);
    void onSleeperTimeout();
};

#endif // MISSIONCONTROL_H
