#include "rotator.h"

Rotator* Rotator::instance = NULL;

Rotator* Rotator::acquire()
{
    return instance ? instance : instance = new Rotator();
}

Rotator::Rotator() : m_slave(NULL), m_policy(Automatic)
{
    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(onResized()));
}

void Rotator::setPolicy(Orientation policy)
{
    m_policy = policy;

#ifdef Q_WS_MAEMO_5
    if (m_slave) switch (m_policy) {
        case Automatic:
            m_slave->setAttribute(Qt::WA_Maemo5AutoOrientation, true);
            m_slave->setAttribute(Qt::WA_Maemo5LandscapeOrientation, false);
            m_slave->setAttribute(Qt::WA_Maemo5PortraitOrientation, false);
            break;
        case Landscape:
            m_slave->setAttribute(Qt::WA_Maemo5AutoOrientation, false);
            m_slave->setAttribute(Qt::WA_Maemo5LandscapeOrientation, true);
            m_slave->setAttribute(Qt::WA_Maemo5PortraitOrientation, false);
            break;
        case Portrait:
            m_slave->setAttribute(Qt::WA_Maemo5AutoOrientation, false);
            m_slave->setAttribute(Qt::WA_Maemo5LandscapeOrientation, false);
            m_slave->setAttribute(Qt::WA_Maemo5PortraitOrientation, true);
            break;
    }
# endif

    onResized();
}

void Rotator::setSlave(QWidget *slave)
{
    m_slave = slave;
}

Rotator::Orientation Rotator::policy()
{
    return m_policy;
}

int Rotator::width()
{
    return w;
}

int Rotator::height()
{
    return h;
}

void Rotator::onResized()
{
    QRect screen = QApplication::desktop()->screenGeometry();

    switch (m_policy) {
        case Portrait:
            // Looks like the window is unconditionally rotatated to landscape
            // whenever entering the task navigator in landscape, so instead of
            // stubbornly drawing the window for portrait (with an ugly result),
            // automatic orientation should be used in this case.
        case Automatic:
            w = screen.width();
            h = screen.height();
            break;
        case Landscape:
            w = qMax(screen.width(), screen.height());
            h = qMin(screen.width(), screen.height());
            break;
    }

    emit rotated(w, h);
}
