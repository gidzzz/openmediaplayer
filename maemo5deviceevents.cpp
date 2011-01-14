#include "maemo5deviceevents.h"

Maemo5DeviceEvents::Maemo5DeviceEvents(QObject *parent) :
    QObject(parent)
{
    this->connectSignals();
}

void Maemo5DeviceEvents::connectSignals()
{
    QDBusConnection::systemBus().connect("",
                                         "/com/nokia/mce/signal",
                                         "com.nokia.mce.signal",
                                         "tklock_mode_ind",
                                         this,
                                         SLOT(onScreenLocked(QString)));
}

void Maemo5DeviceEvents::onScreenLocked(QString state)
{
#ifdef DEBUG
    qDebug() << "Maemo5DeviceEvents: Screen lock status changed: " << state;
#endif
    this->screenState = state;
    if(state == "unlocked")
        emit screenLocked(false);
    else if(state == "locked")
        emit screenLocked(true);
}

bool Maemo5DeviceEvents::isScreenLocked()
{
    if(!this->screenState.isNull()) {
        if(this->screenState == "locked") {
#ifdef DEBUG
            qDebug() << "Maemo5DeviceEvents: Screen is locked";
#endif
            return true;
        } else if(this->screenState == "unlocked") {
#ifdef DEBUG
            qDebug() << "Maemo5DeviceEvents: Screen is unlocked";
#endif
            return false;
        }
    } else {
        // Screen state hasn't changed, so screenState would be a null value
        // Read from sysfs
        QFile tsDisabled("/sys/devices/platform/omap2_mcspi.1/spi1.0/disable_ts");
        tsDisabled.open(QIODevice::ReadOnly);
        QTextStream tsDisabledStream(&tsDisabled);
        int tsState = tsDisabledStream.readLine().toInt();
        tsDisabled.close();
        if(tsState == 1) {
            this->screenState = "locked";
#ifdef DEBUG
            qDebug() << "Maemo5DeviceEvents: Screen is locked";
#endif
            return true;
        } else if(tsState == 0) {
            this->screenState = "unlocked";
#ifdef DEBUG
            qDebug() << "Maemo5DeviceEvents: Screen is unlocked";
#endif
            return false;
        }
    }
}
