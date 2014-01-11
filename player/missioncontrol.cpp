#include "missioncontrol.h"

MissionControl* MissionControl::instance = NULL;

MissionControl* MissionControl::acquire()
{
    return instance ? instance : instance = new MissionControl();
}

MissionControl::MissionControl() :
    m_sleeper(NULL)
{
}

void MissionControl::setFactory(MafwAdapterFactory *factory)
{
    mafwRenderer = factory->getRenderer();
}

Sleeper* MissionControl::sleeper()
{
    if (!m_sleeper) {
        m_sleeper = new Sleeper(this, mafwRenderer);
        connect(m_sleeper, SIGNAL(finished()), this, SLOT(onSleeperTimeout()));
    }

    return m_sleeper;
}

void MissionControl::onSleeperTimeout()
{
    m_sleeper->deleteLater();
    m_sleeper = NULL;
}
