#ifndef MISSIONCONTROL_H
#define MISSIONCONTROL_H

#include <QObject>

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

    Sleeper* m_sleeper;

    MissionControl();

private slots:
    void onSleeperTimeout();
};

#endif // MISSIONCONTROL_H
