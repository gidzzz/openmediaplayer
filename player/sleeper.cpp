#include "sleeper.h"

Sleeper::Sleeper(QObject *parent, MafwRendererAdapter *mafwRenderer) :
    QObject(parent),
    mafwRenderer(mafwRenderer)
{
    endStamp = -1;

    masterTimer = new QTimer(this);
    masterTimer->setSingleShot(true);
    connect(masterTimer, SIGNAL(timeout()), this, SLOT(onTimeout()));

    volumeTimer = new QTimer(this);
    volumeTimer->setSingleShot(true);
    connect(volumeTimer, SIGNAL(timeout()), this, SLOT(stepVolume()));

    connect(mafwRenderer, SIGNAL(propertyChanged(QString,QVariant)), this, SLOT(onPropertyChanged(QString,QVariant)));
}

qint64 Sleeper::end()
{
    return endStamp;
}

void Sleeper::start(int interval, int reduction)
{
    this->reduction = reduction;

    qDebug() << "Setting sleeper timer to" << interval << "ms";

    masterTimer->setInterval(interval);
    masterTimer->start();
    startStamp = QDateTime::currentMSecsSinceEpoch();
    endStamp = startStamp + interval;

    connect(mafwRenderer, SIGNAL(volumeReceived(int,QString)), this, SLOT(onInitialVolumeReceived(int)));
    mafwRenderer->getVolume();
}

void Sleeper::stop()
{
    qDebug() << "Aborting sleeper";

    masterTimer->stop();
    volumeTimer->stop();

    endStamp = -1;

    emit finished();
}

void Sleeper::onInitialVolumeReceived(int volume)
{
    disconnect(mafwRenderer, SIGNAL(volumeReceived(int,QString)), this, SLOT(onInitialVolumeReceived(int)));

    scheduleVolumeStep(volume);
}

void Sleeper::onPropertyChanged(const QString &name, const QVariant &value)
{
    if (name == MAFW_PROPERTY_RENDERER_VOLUME)
        scheduleVolumeStep(value.toInt());
}

void Sleeper::scheduleVolumeStep(int volume)
{
    this->volume = volume;

    if (reduction && volume > 0) {
        qint64 timespan = endStamp - QDateTime::currentMSecsSinceEpoch();
        if (timespan > 0) {
            switch (reduction) {
                case LinearReduction:
                    volumeTimer->setInterval(timespan / volume);
                    break;
                case ExponentialReduction:
                    // The following algorithm is used to determine the timer interval:
                    //     1. Calculate the reference volume for the current moment.
                    //     2. Calculate the scale between the current volume and the reference volume from step 1.
                    //     3. Calculate the reference volume for the current volume minus 1 using the scale from step 2.
                    //     4. Calculate the moment for which the reference volume from step 1 would be equal to the reference volume from step 3.
                    //     5. Calculate the interval as the difference between the moment from step 4 and the current moment.

                    // Exponentially decreasing reference volume can be calculated using the follwing formula:
                    //     v(t) = 100 - (exp(a*t)-1)
                    // Parameter a is constant and adjusts the slope.
                    // Parameter t is the moment for which the reference volume should be calculated.
                    // MAFW accepts volume levels between 0 and 100, so t should be between 0 and ln(100 + 1) / a.

                    const int a = 5;
                    const double tMax = 0.92302410336825;

                    qint64 currentStamp = QDateTime::currentMSecsSinceEpoch();
                    double t = tMax * (currentStamp-startStamp) / (endStamp-startStamp);
                    double referenceVolume = 101 - qExp(a*t);
                    double scale = referenceVolume / volume;
                    double nextReferenceVolume = (volume-1) * scale;
                    double tNext = qLn(101-nextReferenceVolume) / a;
                    int interval = startStamp + (endStamp-startStamp)*(tNext/tMax) - currentStamp;
                    volumeTimer->setInterval(qMax(0, interval));
                    break;
            }
            volumeTimer->start();
            qDebug() << "Current volume level is" << volume << "and next step is in" << volumeTimer->interval() << "ms";
        }
    }
}

void Sleeper::stepVolume()
{
    volume = qMax(0, volume-1);
    mafwRenderer->setVolume(volume);
}

void Sleeper::onTimeout()
{
    endStamp = -1;

    emit finished();

    QString action = QSettings().value("timer/action", "stop-playback").toString();
    qDebug() << "Sleeper countdown finished with action" << action;

    if (action == "stop-playback")
        mafwRenderer->stop();
    else if (action == "pause-playback")
        mafwRenderer->pause();
    else if (action == "close-application")
        QApplication::quit();
}
