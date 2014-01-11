#ifndef SLEEPER_H
#define SLEEPER_H

#include <QObject>

#include <QTimer>
#include <QDateTime>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusArgument>
#include <QApplication>
#include <QSettings>
#include <QtCore>
#include <QDebug>

#include "mafw/mafwrendereradapter.h"

class Sleeper : public QObject
{
    Q_OBJECT

public:
    enum VolumeReduction
    {
        NoReduction = 0,
        LinearReduction,
        ExponentialReduction
    };

    Sleeper(QObject *parent, MafwRendererAdapter *mafwRenderer);

    qint64 end();

    void start(int seconds, int reduction);
    void stop();

private:
    MafwRendererAdapter *mafwRenderer;

    QTimer *masterTimer;
    QTimer *volumeTimer;
    qint64 startStamp;
    qint64 endStamp;
    int reduction;
    int volume;

    void scheduleVolumeStep(int volume);

signals:
    void finished();

private slots:
    void onInitialVolumeReceived(int volume);
    void onPropertyChanged(const QDBusMessage &msg);
    void stepVolume();
    void onTimeout();
};

#endif // SLEEPER_H
