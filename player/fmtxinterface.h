#ifndef FMTXINTERFACE_H
#define FMTXINTERFACE_H

#include <QObject>

class FMTXInterface : public QObject
{
    Q_OBJECT

public:
    enum State
    {
        Enabled,
        Disabled,
        Error,
        Unavailable,
        UnknownState
    };

    enum Startability
    {
        Startable,
        OfflineMode,
        HeadphonesConnected,
        UsbConnected,
        UnknownStartability
    };

    FMTXInterface(QObject *parent);

    uint frequencyMin();
    uint frequencyMax();
    uint frequencyStep();
    uint frequency();
    void setFrequency(uint frequency);

    State state();
    Startability startability();
    void setEnabled(bool enabled);

signals:
    void propertyChanged();

private:
    QVariant property(const QString &name);
    void setProperty(const QString &name, const QVariant &value);

private slots:
    void onPropertyChanged();
};

#endif // FMTXINTERFACE_H
