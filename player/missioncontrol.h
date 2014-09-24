#ifndef MISSIONCONTROL_H
#define MISSIONCONTROL_H

#include <QObject>

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>

#include "lyricsmanager.h"
#include "metadatawatcher.h"
#include "sleeper.h"

#include "mafw/mafwregistryadapter.h"

class MissionControl : public QObject
{
    Q_OBJECT

public:
    static MissionControl* acquire();

    void setRegistry(MafwRegistryAdapter *mafwRegistry);
    void reloadSettings();

    LyricsManager *lyricsManager();
    MetadataWatcher *metadataWatcher();
    Sleeper* sleeper();

private:
    static MissionControl *instance;

    MafwRegistryAdapter *mafwRegistry;
    MafwRendererAdapter *mafwRenderer;

    MetadataWatcher *m_metadataWatcher;
    LyricsManager *m_lyricsManager;
    Sleeper* m_sleeper;

    int mafwState;

    bool pausedByCall;
    bool wasRinging;

    bool wiredHeadsetIsConnected;
    qint64 headsetPauseStamp;
    QTimer *wirelessResumeTimer;

    QString currentArtist;
    QString currentTitle;

    MissionControl();

    void togglePlayback();
    void handlePhoneButton();

private slots:
    void onMediaChanged();
    void onMetadataReady();
    void onMetadataChanged(QString key, QVariant value);

    void onSleeperTimeout();

    void onStatusReceived(MafwPlaylist *, uint, MafwPlayState state, const char *, QString);
    void onStateChanged(int state);

    void onCallStateChanged(QDBusMessage msg);

    void onWirelessHeadsetConnected();
    void onHeadsetConnected();
    void onHeadsetDisconnected();
    void onHeadsetButtonPressed(QDBusMessage msg);
    void updateWiredHeadset();
};

#endif // MISSIONCONTROL_H
