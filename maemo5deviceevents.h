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
    explicit Maemo5DeviceEvents(QObject *parent = 0);
    bool isScreenLocked();

signals:
    void screenLocked(bool);

public slots:

private:
    void connectSignals();
    QString screenState;

private slots:
    void onScreenLocked(QString);

};

#endif // MAEMO5DEVICEEVENTS_H
