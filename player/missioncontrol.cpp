#include "missioncontrol.h"

MissionControl* MissionControl::instance = NULL;

MissionControl* MissionControl::acquire()
{
    return instance ? instance : instance = new MissionControl();
}

MissionControl::MissionControl() :
    m_metadataWatcher(NULL),
    m_lyricsManager(NULL),
    m_sleeper(NULL)
{
}

void MissionControl::setFactory(MafwAdapterFactory *factory)
{
    mafwFactory = factory;

    m_metadataWatcher = new MetadataWatcher(factory);

    connect(m_metadataWatcher, SIGNAL(metadataChanged(QString,QVariant)),
            this, SLOT(onMetadataChanged(QString,QVariant)));
}

void MissionControl::reloadSettings()
{
    if (m_lyricsManager)
        m_lyricsManager->deleteLater();

    // Replacing an old instance with a new one will cause depending objects to
    // be disconnected, but this should not really happen due to NowPlayingWindow
    // also being destroyed when reloading (in MainWindow). Anyway, it still
    // might be a good idea to implement online reloading of plugins.
    m_lyricsManager = QSettings().value("lyrics/enable").toBool()
                    ? new LyricsManager(this)
                    : NULL;
}

LyricsManager* MissionControl::lyricsManager()
{
    return m_lyricsManager;
}

MetadataWatcher* MissionControl::metadataWatcher()
{
    return m_metadataWatcher;
}

Sleeper* MissionControl::sleeper()
{
    if (!m_sleeper) {
        m_sleeper = new Sleeper(this, mafwFactory->getRenderer());
        connect(m_sleeper, SIGNAL(finished()), this, SLOT(onSleeperTimeout()));
    }

    return m_sleeper;
}

void MissionControl::onSleeperTimeout()
{
    m_sleeper->deleteLater();
    m_sleeper = NULL;
}

void MissionControl::onMetadataChanged(QString key, QVariant value)
{
    // Fetch lyrics if enough details become available
    if (!value.isNull()) {
        if (key == MAFW_METADATA_KEY_ARTIST) {
            currentArtist = value.toString();
            if (m_lyricsManager && !currentTitle.isNull())
                m_lyricsManager->fetchLyrics(currentArtist, currentTitle);
        }
        else if (key == MAFW_METADATA_KEY_TITLE) {
            currentTitle = value.toString();
            if (m_lyricsManager && !currentArtist.isNull())
                m_lyricsManager->fetchLyrics(currentArtist, currentTitle);
        }
    }
}
