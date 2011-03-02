/**************************************************************************
    Copyright (C) 2010 Timur Kristof

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

#include "qmaemo5rotator.h"
#if defined(Q_WS_MAEMO_5) || defined(Q_WS_HILDON)
#include <mce/dbus-names.h>
#include <mce/mode-names.h>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>


QMaemo5Rotator::QMaemo5Rotator(RotationBehavior behavior, QWidget *parent)
    : QObject(parent),
    isSetUp(false)
{
    setCurrentBehavior(behavior);
}

QMaemo5Rotator::~QMaemo5Rotator()
{
    QDBusConnection::systemBus().call(QDBusMessage::createMethodCall(MCE_SERVICE, MCE_REQUEST_PATH, MCE_REQUEST_IF, MCE_ACCELEROMETER_DISABLE_REQ));
}

const QMaemo5Rotator::RotationBehavior QMaemo5Rotator::currentBehavior()
{
    return _currentBehavior;
}

const QMaemo5Rotator::Orientation QMaemo5Rotator::currentOrientation()
{
    return _currentOrientation;
}

void QMaemo5Rotator::setCurrentBehavior(QMaemo5Rotator::RotationBehavior value)
{
    if (value == _currentBehavior && isSetUp)
        return;

    isSetUp = true;
    _currentBehavior = value;

    if (value == QMaemo5Rotator::AutomaticBehavior)
    {
        QDBusConnection::systemBus().call(QDBusMessage::createMethodCall(MCE_SERVICE, MCE_REQUEST_PATH, MCE_REQUEST_IF, MCE_ACCELEROMETER_ENABLE_REQ));
        QDBusConnection::systemBus().connect(QString(), MCE_SIGNAL_PATH, MCE_SIGNAL_IF, MCE_DEVICE_ORIENTATION_SIG, this, SLOT(on_orientation_changed(QString)));
    }
    else
    {
        QDBusConnection::systemBus().call(QDBusMessage::createMethodCall(MCE_SERVICE, MCE_REQUEST_PATH, MCE_REQUEST_IF, MCE_ACCELEROMETER_DISABLE_REQ));

        if (value == QMaemo5Rotator::PortraitBehavior)
        {
            setCurrentOrientation(QMaemo5Rotator::PortraitOrientation);
        }
        else
        {
            setCurrentOrientation(QMaemo5Rotator::LandscapeOrientation);
        }
    }
}

void QMaemo5Rotator::setCurrentOrientation(QMaemo5Rotator::Orientation value)
{
    _currentOrientation = value;
    QWidget *par = (QWidget*)parent();

    switch (value)
    {
    case QMaemo5Rotator::PortraitOrientation:
        if (par != NULL)
        {
            par->setAttribute(Qt::WA_Maemo5LandscapeOrientation, false);
            par->setAttribute(Qt::WA_Maemo5PortraitOrientation, true);
        }
        emit portrait();
        break;
    case QMaemo5Rotator::LandscapeOrientation:
        if (par != NULL)
        {
            par->setAttribute(Qt::WA_Maemo5PortraitOrientation, false);
            par->setAttribute(Qt::WA_Maemo5LandscapeOrientation, true);
        }
        emit landscape();

        break;
    }
}

void QMaemo5Rotator::on_orientation_changed(const QString& newOrientation)
{
    if (newOrientation == QLatin1String(MCE_ORIENTATION_PORTRAIT) || newOrientation == QLatin1String(MCE_ORIENTATION_PORTRAIT_INVERTED))
    {
        setCurrentOrientation(QMaemo5Rotator::PortraitOrientation);
    }
    else
    {
        setCurrentOrientation(QMaemo5Rotator::LandscapeOrientation);
    }
    QApplication::desktop()->updateGeometry();
}
#endif
