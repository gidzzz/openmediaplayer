#ifndef MISSIONCONTROL_H
#define MISSIONCONTROL_H

#include <QObject>
#include <QImage>
#include <QCryptographicHash>

#include "sleeper.h"

#include "mafw/mafwadapterfactory.h"

class MissionControl : public QObject
{
    Q_OBJECT

public:
    static MissionControl* acquire();

    Sleeper* sleeper();

    void setFactory(MafwAdapterFactory *factory);

private:
    static MissionControl *instance;

    MafwRendererAdapter *mafwRenderer;
    MafwSourceAdapter *mafwTrackerSource;

    Sleeper* m_sleeper;

    QString currentObjectId;

    MissionControl();

private slots:
    void onSleeperTimeout();

    void onStatusReceived(MafwPlaylist *, uint, MafwPlayState, const char *objectId, QString);
    void onMediaChanged(int, char *objectId);
    void onMetadataChanged(QString metadata, QVariant value);
};

#endif // MISSIONCONTROL_H
