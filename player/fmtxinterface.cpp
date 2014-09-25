#include "fmtxinterface.h"

#include <QDBusConnection>
#include <QDBusReply>

#include "dbus/dbus-shared.h"

#define FMTX_SERVICE "com.nokia.FMTx"
#define FMTX_PATH "/com/nokia/fmtx/default"
#define FMTX_INTERFACE "com.nokia.FMTx.Device"

FMTXInterface::FMTXInterface(QObject *parent) :
    QObject(parent)
{
    QDBusConnection::systemBus().connect(FMTX_SERVICE,
                                         FMTX_PATH,
                                         FMTX_INTERFACE,
                                         "Changed",
                                         this, SLOT(onPropertyChanged()));
}

uint FMTXInterface::frequencyMin()
{
    return property("freq_min").toUInt();
}

uint FMTXInterface::frequencyMax()
{
    return property("freq_max").toUInt();
}

uint FMTXInterface::frequencyStep()
{
    return property("freq_step").toUInt();
}

uint FMTXInterface::frequency()
{
    return property("frequency").toUInt();
}

void FMTXInterface::setFrequency(uint frequency)
{
    setProperty("frequency", frequency);
}

FMTXInterface::State FMTXInterface::state()
{
    const QString state = property("state").toString();

    if (state == "enabled")
        return Enabled;
    if (state == "disabled")
        return Disabled;
    if (state == "error")
        return Error;
    if (state == "n/a")
        return Unavailable;

    // Something went wrong
    return UnknownState;
}

FMTXInterface::Startability FMTXInterface::startability()
{
    const QString state = property("startable").toString();

    if (state == "true")
        return Startable;
    if (state == "Device is in offline mode")
        return OfflineMode;
    if (state == "Headphones are connected")
        return HeadphonesConnected;
    if (state == "Usb device is connected")
        return UsbConnected;

    // Something went wrong
    return UnknownStartability;
}

void FMTXInterface::setEnabled(bool enabled)
{
    // Only these two can be set, other states are read-only
    if (enabled) {
        setProperty("state", "enabled");
    } else {
        setProperty("state", "disabled");
    }
}

QVariant FMTXInterface::property(const QString &name)
{
    QDBusMessage message = QDBusMessage::createMethodCall(FMTX_SERVICE, FMTX_PATH, DBUS_INTERFACE_PROPERTIES, "Get");
    message.setArguments(QList<QVariant>() << DBUS_INTERFACE_PROPERTIES << name);
    return  QDBusReply<QDBusVariant>(QDBusConnection::systemBus().call(message)).value().variant();
    // NOTE: Accessing properties using QDBusInterface crashes fmtxd for some reason
}

void FMTXInterface::setProperty(const QString &name, const QVariant &value)
{
    QDBusMessage message = QDBusMessage::createMethodCall(FMTX_SERVICE, FMTX_PATH, DBUS_INTERFACE_PROPERTIES, "Set");
    message.setArguments(QList<QVariant>() << DBUS_INTERFACE_PROPERTIES << name << QVariant::fromValue(QDBusVariant(value)));
    QDBusConnection::systemBus().call(message, QDBus::NoBlock);
    // NOTE: Accessing properties using QDBusInterface crashes fmtxd for some reason
}

void FMTXInterface::onPropertyChanged()
{
    emit propertyChanged();
}
