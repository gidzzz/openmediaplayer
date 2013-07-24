#ifndef MAEMO5DEVICEEVENTS_H
#define MAEMO5DEVICEEVENTS_H

#include <QObject>
#include <QtDBus>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDebug>

class Maemo5DeviceEvents : public QObject
{
    Q_OBJECT

public:
    static Maemo5DeviceEvents* acquire();
    bool isScreenLocked();

signals:
    void screenLocked(bool);

private:
    static Maemo5DeviceEvents *instance;
    Maemo5DeviceEvents();
    void connectSignals();
    QString screenState;

private slots:
    void onScreenLocked(QString);

};

#endif // MAEMO5DEVICEEVENTS_H
