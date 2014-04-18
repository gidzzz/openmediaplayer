/**************************************************************************
    This file is part of Open MediaPlayer
    Copyright (C) 2010-2011 Mohammad Abu-Garbeyyeh

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

#include "maemo5deviceevents.h"

Maemo5DeviceEvents* Maemo5DeviceEvents::instance = NULL;

Maemo5DeviceEvents* Maemo5DeviceEvents::acquire()
{
    return instance ? instance : instance = new Maemo5DeviceEvents();
}

Maemo5DeviceEvents::Maemo5DeviceEvents()
{
    this->screenState = "unlocked";
    this->connectSignals();
    QDBusPendingCall call = QDBusConnection::systemBus().asyncCall(QDBusMessage::createMethodCall("com.nokia.mce", "/com/nokia/mce/request", "com.nokia.mce.request", "get_tklock_mode"));
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), this, SLOT(watcherFinished(QDBusPendingCallWatcher*)));
}

void Maemo5DeviceEvents::watcherFinished(QDBusPendingCallWatcher *watcher)
{
    QDBusPendingReply<QString> reply = *watcher;
    QString state = reply;
    watcher->deleteLater();
    onScreenLocked(state);
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
    if (state.endsWith("unlocked"))
        emit screenLocked(false);
    else
        emit screenLocked(true);
}

bool Maemo5DeviceEvents::isScreenLocked()
{
    if (this->screenState.endsWith("unlocked")) {
#ifdef DEBUG
        qDebug() << "Maemo5DeviceEvents: Screen is unlocked";
#endif
        return false;
    } else {
#ifdef DEBUG
        qDebug() << "Maemo5DeviceEvents: Screen is locked";
#endif
        return true;
    }
}
